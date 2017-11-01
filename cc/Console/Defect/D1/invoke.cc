// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/Gpio/Function.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>
#include <iostream>

static void defect(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto span = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;

    namespace Function = Rpi::Gpio::Function ;
    
    auto gpio = rpi->page<Rpi::Register::Gpio::PageNo>() ;
    auto status = rpi->at<Rpi::Register::Gpio::Event::   Status0>() ;
    auto detect = rpi->at<Rpi::Register::Gpio::Event::AsyncFall0>() ;
    Rpi::Timer timer(rpi) ;
    
    auto mask = 1u << pin.value() ;

    // setup: mode=In output=Low event=AsyncFall status=Clear
    Function::set(gpio,pin,Function::Type::In) ;
    rpi->at<Rpi::Register::Gpio::Output::Clear0>().poke(mask) ;
    detect += mask ;
    status.poke(mask) ;
    
    // precondition: pin must have been pulled to High by resistor!
    
    auto t0 = timer.cLo() ;
    // create Low pulse on pin thru GPIO mode switch
    Function::set(gpio,pin,Function::Type::Out) ;
    Function::set(gpio,pin,Function::Type:: In) ;
    auto t1 = timer.cLo() ;
    detect -= mask ;

    if (0 == (status.peek() & mask))
    {
	std::cout << "error: no event detected or already cleared "
		  << "(" << (t1-t0) << "us)\n" ;
	return ;
    }

    while (0 != (status.peek() & mask))
    {
	t1 = timer.cLo() ;
	if (t1 - t0 > span)
	{
	    // problem: event detect status flag got cleared
	    std::cout << "defect not reproduced (" << (t1-t0) << "us)\n" ;
	    return ;
	}
    }

    auto t2 = timer.cLo() ;
    std::cout << "defect reproduced (" << (t1-t0) << '+' << (t2-t1) << "us)\n" ;
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
