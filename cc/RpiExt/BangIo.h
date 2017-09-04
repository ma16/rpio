// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_BangIo_h
#define INCLUDE_RpiExt_BangIo_h

#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>

namespace RpiExt {

struct BangIo
{
    void mode(Rpi::Pin pin,Rpi::Gpio::Mode mode)
    {
	this->gpio.setMode(pin,mode) ;
    }

    uint32_t recent() const
    {
	return this->t ;
    }

    void reset(uint32_t pins)
    {
	this->gpio.setOutput<Rpi::Gpio::Output::Lo>(pins) ;
    }
    
    void set(uint32_t pins)
    {
	this->gpio.setOutput<Rpi::Gpio::Output::Hi>(pins) ;
    }

    void sleep(uint32_t span)
    {
	if (span > 0)
	{
	    auto t0 = this->t = this->counter.clock() ;
	    while (this->t - t0 < span)
		this->t = this->counter.clock() ;
	}
    }

    uint32_t time()
    {
	return this->t = this->counter.clock() ;
    }

    void wait(uint32_t t0,uint32_t span)
    {
	while (this->t - t0 < span) 
	    this->t = this->counter.clock() ;
    }
    
    uint32_t waitFor(uint32_t t0,uint32_t span,uint32_t mask,uint32_t cond)
    {
	do
	{
	    this->l = this->gpio.getLevels() ;
	    if (cond == (l & mask))
	    {
		auto t = this->t ;
		this->t = this->counter.clock() ;
		return t ;
	    }
	    this->t = this->counter.clock() ;
	}
	while (this->t - t0 <= span) ;
	return this->t ;
    }

    BangIo(Rpi::Peripheral *rpi)
	: counter(Rpi::Counter(rpi))
	, gpio      (Rpi::Gpio(rpi))
	, t        (counter.clock())
	{ }

private:
    
    Rpi::Counter counter ; Rpi::Gpio gpio ;

    uint32_t t ; // last read time-stamp
    uint32_t l ; // last read GPIO level
} ; }
	
#endif // INCLUDE_RpiExt_BangIo_h
