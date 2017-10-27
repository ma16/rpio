// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/GpioOld.h>
#include <Rpi/Gpio/Function.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>
#include <iostream>

static void recover(Rpi::GpioOld *gpio,uint32_t mask)
{
    gpio->enable(mask,Rpi::GpioOld::Event::Fall,false) ;
    gpio->setOutput(mask,Rpi::GpioOld::Output::Lo) ;
}

static void recover(Rpi::Gpio::Function *function,Rpi::Pin pin)
{
  function->set(pin,Rpi::Gpio::Function::Mode::In) ;
}

static void defect(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto span = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    
    Rpi::GpioOld gpio(rpi) ;
    Rpi::Gpio::Function function(rpi) ;
    Rpi::Timer timer(rpi) ;
    auto mask = 1u << pin.value() ;
    recover(&gpio,mask) ;
    recover(&function,pin) ;
    
    function.set(pin,Rpi::Gpio::Function::Mode::Out) ;
    gpio.enable(mask,Rpi::GpioOld::Event::Fall,true) ;
    gpio.reset(mask) ;

    auto t0 = timer.cLo() ;
    function.set(pin,Rpi::Gpio::Function::Mode:: In) ;
    function.set(pin,Rpi::Gpio::Function::Mode::Out) ;
    function.set(pin,Rpi::Gpio::Function::Mode:: In) ;
    auto t1 = timer.cLo() ;
    
    if (0 == (gpio.getEvents() & mask))
    {
	std::cout << "error: no event detected or already cleared "
		  << "(" << (t1-t0) << "us)\n" ;
	recover(&gpio,mask) ;
	recover(&function,pin) ;
	return ;
    }

    while (0 != (gpio.getEvents() & mask))
    {
	t1 = timer.cLo() ;
	if (t1 - t0 > span)
	{
	    std::cout << "defect not reproduced (" << (t1-t0) << "us)\n" ;
	    recover(&gpio,mask) ;
	    recover(&function,pin) ;
	    return ;
	}
    }

    auto t2 = timer.cLo() ;
    std::cout << "defect reproduced (" << (t1-t0) << '+' << (t2-t1) << "us)\n" ;
    recover(&gpio,mask) ;
    recover(&function,pin) ;
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
