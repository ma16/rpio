// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Master.h"

using namespace Device::Ds18x20::Bang::OneWire ;

bool Master::init()
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    
    // tx: Reset-Pulse
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    // ...assumes the configured output level is Low
    // ...note, errors must not be thrown as long as Out or Events enabled
    this->io.sleep(this->timing.rstl) ;
    auto v = this->intr.status() ;
    // https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=192908
    this->intr.disable(v) ;
    this->io.detect(this->busPin,Rpi::Gpio::Event::Fall) ;
    // the event status bit gets immediately set (todo: why?)
    this->io.events(this->pinMask) ;
    auto t2 = this->io.recent() ;
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->io.time() ;
    // ...if we got suspended, t3 and following time-stamps may lay
    // even behind the end of the Presence-Pulse (if there was any)

    auto isPresent = 0 != this->io.waitForEvent(t3,
						this->timing.pdhigh.max,
						this->pinMask) ;
    auto t4 = this->io.recent() ;
    auto t5 = this->io.time() ;
    this->io.detect(this->busPin,Rpi::Gpio::Event::Fall,false) ;
    this->io.events(this->pinMask) ; // reset late events
    this->intr.enable(v) ;

    if (isPresent)
    {
	if (t4 == t3)
	    throw Error(Error::Type::Retry,__LINE__) ;
	if (t5 - t2 < this->timing.pdhigh.min)
	    throw Error(Error::Type::Timing,__LINE__) ;

	// rx: wait for LH-edge (end of Presence-Pulse)
	this->io.waitForLevel(t5,this->timing.pdlow.max,
			      this->pinMask,/*High*/this->pinMask) ;
	auto t7 = this->io.recent() ;
	if (t7 - t4 < this->timing.pdlow.min)
	    throw Error(Error::Type::Timing,__LINE__) ;
	
	// note: we don't check against max values at the moment
	// because the results may be false positives (due to delays
	// when recording the time-stamps). in order to get a better
	// grip we may want to check the gap when recording the time
	// before and after every (!) edge.
    }
    
    // rx: wait for end of Presence-Pulse cycle
    this->io.wait(t3,this->timing.rsth) ;

    return isPresent ;
}

void Master::write(bool bit) 
{
    //  t0 t1 t2 t3
    // ---+     +---
    //    |     |
    //    +-----+

    // tx: short or long Bit Pulse
    auto t0 = this->io.time() ;
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->io.time() ;
    auto range = bit ? this->timing.low1 : this->timing.low0 ;
    this->io.wait(t1,range.min) ; 
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->io.time() ; 
    if (t3 - t0 > range.max) 
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->io.wait(t3,this->timing.rec) ;
	
    // rx: wait for end of cycle
    this->io.wait(t1,this->timing.slot.min) ;
}

bool Master::read()
{
    //  t0 t1 t2 t3 t4 t5
    // ---+     +-----+---
    //    |     |     |   
    //    +-----+-----+

    // tx: initiate Read-Time-Slot
    auto t0 = this->io.time() ; 
    this->io.mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->io.time() ; 
    this->io.wait(t1,this->timing.rinit) ;
    // the device keeps holding the wire low in order to "send" a
    // 0-bit; there is no LH-HL gap as long as the pulse to initiate
    // the Read-Time-Slot is long enough
    this->io.mode(this->busPin,Rpi::Gpio::Mode::In) ;
    this->io.sleep(this->timing.rrc) ;
    auto sample_t3 = 0 != (this->pinMask & this->io.levels()) ;
    auto t3 = this->io.time() ; 
    // ...if we got suspended, t3 and following time-stamps may lay
    // far behind the end of the 0-bit Pulse (if there was any)
    if (t3 - t0 > this->timing.rdv) 
	throw Error(Error::Type::Retry,__LINE__) ;
    // ...this does also cover t3 - t0 > rstl

    // rx: wait for LH edge 
    this->io.waitForLevel(t1,this->timing.slot.max,
			  this->pinMask,/*High*/this->pinMask) ;
    auto t5 = this->io.recent() ;
    auto timeout = t5 - t1 >= this->timing.slot.max ;
    if (timeout)
	throw Error(Error::Type::Retry,__LINE__) ;

    auto sample_rdv = (t5 - t1) <= this->timing.rdv ;
    if (sample_t3 != sample_rdv)
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->io.wait(t5,this->timing.rec) ;
	
    // rx: wait for end of cycle
    this->io.wait(t1,this->timing.slot.min) ;

    return sample_t3 ;
}
