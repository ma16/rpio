// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_BangIo_h
#define INCLUDE_RpiExt_BangIo_h

#include <arm/arm.h>
#include <Rpi/ArmTimer.h>
#include <Rpi/GpioOld.h>
#include <Rpi/Gpio/Function.h>

namespace RpiExt {

struct BangIo
{
    void detect(Rpi::Pin pin,Rpi::GpioOld::Event event,bool enable=true)
    {
	this->gpio.enable(pin,event,enable) ;
    }

    uint32_t events(uint32_t mask)
    {
	auto raised = mask & this->gpio.getEvents() ;
	if (raised != 0)
	    this->gpio.reset(raised) ;
	return raised ;
    }

    uint32_t levels()
    {
	return this->gpio.getLevels() ;
    }

    void mode(Rpi::Pin pin,Rpi::Gpio::Function::Mode mode)
    {
	this->function.set(pin,mode) ;
    }

    uint32_t recent() const
    {
	return this->t ;
    }

    void reset(uint32_t pins)
    {
	this->gpio.setOutput<Rpi::GpioOld::Output::Lo>(pins) ;
    }
    
    void set(uint32_t pins)
    {
	this->gpio.setOutput<Rpi::GpioOld::Output::Hi>(pins) ;
    }

    void sleep(uint32_t span)
    {
	if (span > 0)
	{
	    auto t0 = this->t = this->timer.counter().read() ;
	    while (this->t - t0 < span)
		this->t = this->timer.counter().read() ;
	}
    }

    uint32_t time()
    {
	return this->t = this->timer.counter().read() ;
    }

    void wait(uint32_t t0,uint32_t span)
    {
	while (this->t - t0 < span) 
	    this->t = this->timer.counter().read() ;
    }
    
    uint32_t waitForEvent(uint32_t t0,uint32_t span,uint32_t mask)
    {
	do
	{
	    arm::dmb() ; // since we got strange values
	    auto raised = mask & this->gpio.getEvents() ;
	    if (raised != 0)
	    {
		this->gpio.reset(raised) ;
		return raised ;
	    }
	    arm::dmb() ; // since we got strange values
	    this->t = this->timer.counter().read() ;
	}
	while (this->t - t0 <= span) ;
	return 0 ;
    }

    uint32_t waitForLevel(uint32_t t0,uint32_t span,uint32_t mask,uint32_t cond)
    {
	do
	{
	    arm::dmb() ; // since we got strange values
	    this->l = this->gpio.getLevels() ;
	    arm::dmb() ; // since we got strange values
	    if (cond == (l & mask))
	    {
		auto tx = this->t ;
		this->t = this->timer.counter().read() ;
		return tx ;
	    }
	    this->t = this->timer.counter().read() ;
	}
	while (this->t - t0 <= span) ;
	return this->t ;
	// [todo] we might want to record also the gap between two
	// subsequent timings and return the maximum of these values
	// in order to ease the detection of involuntary context
	// switches or other kinds of thread interruption
    }

    BangIo(Rpi::Peripheral *rpi)
	: timer         (Rpi::ArmTimer(rpi))
	, gpio           (Rpi::GpioOld(rpi))
	, function(Rpi::Gpio::Function(rpi))
	, t         (timer.counter().read())
	{ }

private:
    
    Rpi::ArmTimer timer ; Rpi::GpioOld gpio ; Rpi::Gpio::Function function ;

    uint32_t t ; // last read time-stamp
    uint32_t l ; // last read GPIO level
} ; }
	
#endif // INCLUDE_RpiExt_BangIo_h
