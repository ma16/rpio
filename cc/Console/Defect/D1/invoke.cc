// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/Gpio.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>

static void recover(Rpi::Gpio *gpio,uint32_t mask)
{
    gpio->enable(mask,Rpi::Gpio::Event::Fall,false) ;
    gpio->setOutput(mask,Rpi::Gpio::Output::Lo) ;
    gpio->setMode(mask,Rpi::Gpio::Mode::In) ;
}

static void defect(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto span = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    
    Rpi::Gpio gpio(rpi) ;
    Rpi::Timer timer(rpi) ;
    auto mask = 1u << pin.value() ;
    recover(&gpio,mask) ;
    
    gpio.setMode(mask,Rpi::Gpio::Mode::Out) ;
    gpio.enable(mask,Rpi::Gpio::Event::Fall,true) ;
    gpio.reset(mask) ;

    auto t0 = timer.cLo() ;
    gpio.setMode(mask,Rpi::Gpio::Mode::In) ;
    gpio.setMode(mask,Rpi::Gpio::Mode::Out) ;
    gpio.setMode(mask,Rpi::Gpio::Mode::In) ;
    auto t1 = timer.cLo() ;
    
    if (0 == (gpio.getEvents() & mask))
    {
	std::cout << "error: no event detected or already cleared "
		  << "(" << (t1-t0) << "us)\n" ;
	return recover(&gpio,mask) ;
    }

    while (0 != (gpio.getEvents() & mask))
    {
	t1 = timer.cLo() ;
	if (t1 - t0 > span)
	{
	    std::cout << "defect not reproduced (" << (t1-t0) << "us)\n" ;
	    return recover(&gpio,mask) ;
	}
    }

    auto t2 = timer.cLo() ;
    std::cout << "defect reproduced (" << (t1-t0) << '+' << (t2-t1) << "us)\n" ;
    recover(&gpio,mask) ;
}

static void help()
{
    std::cout
	<< "Defect Description:\n"
	<< "The Raspbian Kernel resets the GPIO Event Detect Status Register\n"
	<< "https://www.raspberrypi.org/forums/viewtopic.php?f=66&t=192908\n"
	<< "This defect is _NOT_ reproducible.\n"
	<< "Neither here nor in the original source code.\n"
	<< '\n'
	<< "Arguments: PIN TIMEOUT\n"
	<< '\n'
	<< "PIN: GPIO pin number to use\n"
	<< "TIMEOUT: time in us to poll bit and check if cleared\n"
	;
}

void Console::Defect::D1::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
	return help() ;
    defect(rpi,argL) ;
}
