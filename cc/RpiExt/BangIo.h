// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_BangIo_h
#define INCLUDE_RpiExt_BangIo_h

#include <arm/arm.h>
#include <Rpi/ArmTimer.h>
#include <Rpi/Gpio/Event.h>
#include <Rpi/Gpio/Function.h>
#include <Rpi/Gpio/Input.h>
#include <Rpi/Gpio/Output.h>

namespace RpiExt {

struct BangIo
{
    void detect(Rpi::Pin pin,Rpi::Gpio::Event::Type event,bool enable=true)
    {
	this->event.enable(pin,event,enable) ;
	// [todo] better mask instead of pin
    }

    uint32_t events(uint32_t mask)
    {
	auto raised = mask & this->event.status0().read() ;
	if (raised != 0)
	    this->event.status0().write(raised) ;
	return raised ;
    }

    uint32_t levels()
    {
	return this->input.bank0().read() ;
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
	this->output.clear().write(pins) ;
    }
    
    void set(uint32_t pins)
    {
	this->output.raise().write(pins) ;
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
	    auto raised = mask & this->event.status0().read() ;
	    if (raised != 0)
	    {
		this->event.status0().write(raised) ;
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
	    this->l = this->input.bank0().read() ;
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
	, event      (Rpi::Gpio::Event(rpi))
	, function(Rpi::Gpio::Function(rpi))
	, input      (Rpi::Gpio::Input(rpi))
	, output    (Rpi::Gpio::Output(rpi))
	, t         (timer.counter().read())
	{ }

private:
    
    Rpi::ArmTimer timer ;

    Rpi::Gpio::Event       event ;
    Rpi::Gpio::Function function ;
    Rpi::Gpio::Input       input ;
    Rpi::Gpio::Output     output ;

    uint32_t t ; // last read time-stamp
    uint32_t l ; // last read GPIO level
} ; }
	
#endif // INCLUDE_RpiExt_BangIo_h
