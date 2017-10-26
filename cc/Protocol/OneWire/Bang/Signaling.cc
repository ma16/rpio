// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"
#include "Signaling.h"

using namespace Protocol::OneWire::Bang ;

// defect:
// https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=192908
#define DEFECT_D1 1
// don't disable interrupts anymore since the problem does no longer
// occur.

bool Signaling::init()
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    
    // Reset-Pulse
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::Out) ;
    // ...assumes the configured output level is Low
    // ...note, errors must not be thrown as long as Out or Events enabled
    this->master->io.sleep(this->master->timing.resetPulse_min) ;
    // ...[todo] this may be a good time to voluntary suspend the thread
#if DEFECT_D1    
    auto v = this->master->intr.status() ;
    this->master->intr.disable(v) ;
#endif    
    this->master->io.detect(this->master->pin,Rpi::GpioOld::Event::Fall) ;
    // [defect] The event status flag gets immediately* raised regardless
    // whether Fall or AsyncFall is used.
    // (*) mostly in the immediate query, or the query thereafter;
    // however not always (not exactly reproducible). Still, with
    // AsyncFall mostly in the immediate query (and not later).
    this->master->io.events(this->master->mask) ;
    auto t2 = this->master->io.recent() ;
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::In) ;
    auto t3 = this->master->io.time() ;
    // ...if we got suspended, t3 and following time-stamps may lay
    // even behind the end of the Presence-Pulse (if there was any)
    auto isPresent = 0 !=
	this->master->io.waitForEvent(t3,
				      this->master->timing.presenceIdle_max,
				      this->master->mask) ;
    auto t4 = this->master->io.recent() ;
    auto t5 = this->master->io.time() ;
    this->master->io.detect(this->master->pin,Rpi::GpioOld::Event::Fall,false) ;
    this->master->io.events(this->master->mask) ; // reset late events
#if DEFECT_D1
    this->master->intr.enable(v) ;
#endif
    if (isPresent)
    {
	if (t4 == t3)
	    throw Error(Error::Type::Retry,__LINE__) ;
	if (t5 - t2 < this->master->timing.presenceIdle_min)
	    throw Error(Error::Type::Timing,__LINE__) ;

	// rx: wait for LH-edge (end of Presence-Pulse)
	this->master->io.waitForLevel(t5,
				      this->master->timing.presencePulse_max,
				      this->master->mask,
				      /*High*/this->master->mask) ;
	auto t7 = this->master->io.recent() ;
	if (t7 - t4 < this->master->timing.presencePulse_min)
	    throw Error(Error::Type::Timing,__LINE__) ;
    }
    
    // end of init. sequence
    this->master->io.wait(t3,this->master->timing.presenceFrame_max) ;

    return isPresent ;
}

bool Signaling::read(bool busy)
{
    //  t0 t1 t2 t3 t4 t5
    // ---+     +-----+---
    //    |     |     |   
    //    +-----+-----+

    // initiate Read-Time-Slot
    auto t0 = this->master->io.time() ; 
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::Out) ;
    auto t1 = this->master->io.time() ; 
    this->master->io.wait(t1,this->master->timing.init_min) ;
    // the device keeps holding the wire low in order to "send" a
    // 0-bit; there is no LH-HL gap as long as the pulse to initiate
    // the Read-Time-Slot is long enough
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::In) ;
    this->master->io.sleep(this->master->timing.rc_max) ;
    auto sample_t3 = 0 != (this->master->mask & this->master->io.levels()) ;
    auto t3 = this->master->io.time() ; 
    // ...if we got suspended, t3 and following time-stamps may lay
    // far behind the end of the 0-bit Pulse (if there was any)
    if (busy && t3-t0 > this->master->timing.init_max) 
	throw Error(Error::Type::Reset,__LINE__) ;
    if (t3 - t0 > this->master->timing.rdv_min) 
	throw Error(Error::Type::Retry,__LINE__) ;
    
    // wait for LH edge (depending on the time we got a 1-bit or 0-bit)
    this->master->io.waitForLevel(t1,
				  this->master->timing.slot_max,
				  this->master->mask,
				  /*High*/this->master->mask) ;
    auto t5 = this->master->io.recent() ;
    // minimum idle time after LH edge
    this->master->io.wait(t5,this->master->timing.rec_min) ;
    auto timeout = t5 - t1 >= this->master->timing.slot_max ;
    if (timeout)
	throw Error(Error::Type::Retry,__LINE__) ;
    auto sample_rdv = (t5 - t1) <= this->master->timing.rdv_min ;
    if (sample_t3 != sample_rdv)
	throw Error(Error::Type::Retry,__LINE__) ;
    // wait for end of time-slot period
    this->master->io.wait(t1,this->master->timing.slot_min) ;
    return sample_t3 ;
}

void Signaling::write(bool bit) 
{
    //  t0 t1 t2 t3
    // ---+     +---
    //    |     |
    //    +-----+

    // tx: short or long Bit Pulse
    auto t0 = this->master->io.time() ;
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::Out) ;
    auto t1 = this->master->io.time() ;
    auto min = bit
	? this->master->timing.write_1_min
	: this->master->timing.write_0_min ;
    this->master->io.wait(t1,min) ; 
    this->master->io.mode(this->master->pin,Rpi::GpioOld::Mode::In) ;
    auto t3 = this->master->io.time() ; 
    auto max = bit
	? this->master->timing.write_1_max
	: this->master->timing.write_0_max ;
    if (t3 - t0 > max) 
	throw Error(Error::Type::Retry,__LINE__) ;
    // wait for end of time-slot period
    this->master->io.wait(t1,this->master->timing.slot_min) ;
}
