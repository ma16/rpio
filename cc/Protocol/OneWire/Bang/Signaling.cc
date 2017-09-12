// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Signaling.h"

using namespace Protocol::OneWire::Bang ;

bool Signaling::init()
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    
    // tx: Reset-Pulse
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::Out) ;
    // ...assumes the configured output level is Low
    // ...note, errors must not be thrown as long as Out or Events enabled
    this->master->io.sleep(this->master->timing.rstl) ;
    auto v = this->master->intr.status() ;
    // https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=192908
    this->master->intr.disable(v) ;
    this->master->io.detect(this->master->busPin,Rpi::Gpio::Event::Fall) ;
    // the event status bit gets immediately set (todo: why?)
    this->master->io.events(this->master->pinMask) ;
    auto t2 = this->master->io.recent() ;
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->master->io.time() ;
    // ...if we got suspended, t3 and following time-stamps may lay
    // even behind the end of the Presence-Pulse (if there was any)

    auto isPresent = 0 !=
      this->master->io.waitForEvent(t3,
				    this->master->timing.pdhigh.max,
				    this->master->pinMask) ;
    auto t4 = this->master->io.recent() ;
    auto t5 = this->master->io.time() ;
    this->master->io.detect(this->master->busPin,Rpi::Gpio::Event::Fall,false) ;
    this->master->io.events(this->master->pinMask) ; // reset late events
    this->master->intr.enable(v) ;

    if (isPresent)
    {
	if (t4 == t3)
	    throw Error(Error::Type::Retry,__LINE__) ;
	if (t5 - t2 < this->master->timing.pdhigh.min)
	    throw Error(Error::Type::Timing,__LINE__) ;

	// rx: wait for LH-edge (end of Presence-Pulse)
	this->master->io.waitForLevel(t5,this->master->timing.pdlow.max,
				      this->master->pinMask,
				      /*High*/this->master->pinMask) ;
	auto t7 = this->master->io.recent() ;
	if (t7 - t4 < this->master->timing.pdlow.min)
	    throw Error(Error::Type::Timing,__LINE__) ;
	
	// note: we don't check against max values at the moment
	// because the results may be false positives (due to delays
	// when recording the time-stamps). in order to get a better
	// grip we may want to check the gap when recording the time
	// before and after every (!) edge.
    }
    
    // rx: wait for end of Presence-Pulse cycle
    this->master->io.wait(t3,this->master->timing.rsth) ;

    return isPresent ;
}

void Signaling::write(bool bit) 
{
    //  t0 t1 t2 t3
    // ---+     +---
    //    |     |
    //    +-----+

    // tx: short or long Bit Pulse
    auto t0 = this->master->io.time() ;
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->master->io.time() ;
    auto range = bit ? this->master->timing.low1 : this->master->timing.low0 ;
    this->master->io.wait(t1,range.min) ; 
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = this->master->io.time() ; 
    if (t3 - t0 > range.max) 
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->master->io.wait(t3,this->master->timing.rec) ;
	
    // rx: wait for end of cycle
    this->master->io.wait(t1,this->master->timing.slot.min) ;
}

bool Signaling::read()
{
    //  t0 t1 t2 t3 t4 t5
    // ---+     +-----+---
    //    |     |     |   
    //    +-----+-----+

    // tx: initiate Read-Time-Slot
    auto t0 = this->master->io.time() ; 
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = this->master->io.time() ; 
    this->master->io.wait(t1,this->master->timing.rinit) ;
    // the device keeps holding the wire low in order to "send" a
    // 0-bit; there is no LH-HL gap as long as the pulse to initiate
    // the Read-Time-Slot is long enough
    this->master->io.mode(this->master->busPin,Rpi::Gpio::Mode::In) ;
    this->master->io.sleep(this->master->timing.rrc) ;
    auto sample_t3 = 0 != (this->master->pinMask & this->master->io.levels()) ;
    auto t3 = this->master->io.time() ; 
    // ...if we got suspended, t3 and following time-stamps may lay
    // far behind the end of the 0-bit Pulse (if there was any)
    if (t3 - t0 > this->master->timing.rdv) 
	throw Error(Error::Type::Retry,__LINE__) ;
    // ...this does also cover t3 - t0 > rstl

    // rx: wait for LH edge 
    this->master->io.waitForLevel(t1,this->master->timing.slot.max,
				  this->master->pinMask,
				  /*High*/this->master->pinMask) ;
    auto t5 = this->master->io.recent() ;
    auto timeout = t5 - t1 >= this->master->timing.slot.max ;
    if (timeout)
	throw Error(Error::Type::Retry,__LINE__) ;

    auto sample_rdv = (t5 - t1) <= this->master->timing.rdv ;
    if (sample_t3 != sample_rdv)
	throw Error(Error::Type::Retry,__LINE__) ;

    // minimum recovery time (after LH edge)
    this->master->io.wait(t5,this->master->timing.rec) ;
	
    // rx: wait for end of cycle
    this->master->io.wait(t1,this->master->timing.slot.min) ;

    return sample_t3 ;
}
