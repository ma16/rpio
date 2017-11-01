// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_BangIo_h
#define INCLUDE_RpiExt_BangIo_h

#include <arm/arm.h>
#include <Rpi/ArmTimer.h>
#include <Rpi/Gpio/Function.h>

namespace RpiExt {

struct BangIo
{
    void detect(Rpi::Pin pin,Rpi::Register::Gpio::Event::Type type,bool enable=true)
    {
	// [todo] would be faster...
	// * if 32-bit mask (instead of pin);
	// * if dedicated enable/disable methods (instead of (...bool enable)
	auto select = [this,type]
	{
	    namespace Register = Rpi::Register::Gpio::Event ;
	    using Type = Register::Type ;
	    switch (type)
	    {
	    case Type::     Rise: return this->gpio.at<Register::     Rise0>().value() ;
	    case Type::     Fall: return this->gpio.at<Register::     Fall0>().value() ;
	    case Type::     High: return this->gpio.at<Register::     High0>().value() ;
	    case Type::      Low: return this->gpio.at<Register::      Low0>().value() ;
	    case Type::AsyncRise: return this->gpio.at<Register::AsyncRise0>().value() ;
	    case Type::AsyncFall: return this->gpio.at<Register::AsyncFall0>().value() ;
	    }
	    abort() ;
	    // ...[todo] switch() is far beyond optimal
	    // ...(use template<Type> function instead)
	} ;
	auto p = select() ;
	if (enable) (*p) |=   1u << pin.value()  ;
	else        (*p) &= ~(1u << pin.value()) ;
    }

    uint32_t events(uint32_t mask)
    {
	auto status = this->gpio.at<Rpi::Register::Gpio::Event::Status0>().value() ;
	auto events = mask & (*status) ;
	if (events != 0)
	    (*status) = events ;
	return events ;
    }

    uint32_t levels()
    {
	auto input = this->gpio.at<Rpi::Register::Gpio::Input::Bank0>().value() ;
	return (*input) ;
    }

    void mode(Rpi::Pin pin,Rpi::Gpio::Function::Type mode)
    {
	Rpi::Gpio::Function::set(this->gpio,pin,mode) ;
    }

    uint32_t recent() const
    {
	return this->t ;
    }

    void reset(uint32_t pins)
    {
	auto clear = this->gpio.at<Rpi::Register::Gpio::Output::Clear0>().value() ;
	(*clear) = pins ;
    }
    
    void set(uint32_t pins)
    {
	auto raise = this->gpio.at<Rpi::Register::Gpio::Output::Raise0>().value() ;
	(*raise) = pins ;
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
	auto status = this->gpio.at<Rpi::Register::Gpio::Event::Status0>().value() ;
	do
	{
	    arm::dmb() ; // since we got strange values
	    auto events = mask & (*status) ;
	    if (events != 0)
	    {
		(*status) = events ;
		return events ;
	    }
	    arm::dmb() ; // since we got strange values
	    this->t = this->timer.counter().read() ;
	}
	while (this->t - t0 <= span) ;
	return 0 ;
    }

    uint32_t waitForLevel(uint32_t t0,uint32_t span,uint32_t mask,uint32_t cond)
    {
	auto input = this->gpio.at<Rpi::Register::Gpio::Input::Bank0>().value() ;
	do
	{
	    arm::dmb() ; // since we got strange values
	    this->l = (*input) ;
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
	, gpio(rpi->page<Rpi::Register::Gpio::PageNo>())
	  
	, t         (timer.counter().read())
	{ }

private:
    
    Rpi::ArmTimer timer ;

    Rpi::Register::Base<Rpi::Register::Gpio::PageNo> gpio ;

    uint32_t t ; // last read time-stamp
    uint32_t l ; // last read GPIO level
} ; }
	
#endif // INCLUDE_RpiExt_BangIo_h
