// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/Gpio/Event.h>
#include <Rpi/Gpio/Function.h>
#include <Rpi/Gpio/Output.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>
#include <iostream>

static void defect(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto span = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    
    Rpi::Gpio::Event       event(rpi) ;
    Rpi::Gpio::Function function(rpi) ;
    Rpi::Gpio::Output     output(rpi) ;
    Rpi::Timer             timer(rpi) ;
    
    auto mask = 1u << pin.value() ;

    // setup: mode=In output=Low event=AsyncFall status=Clear
    function.set(pin,Rpi::Gpio::Function::Mode::In) ;
    output.clear().write(mask) ;
    event.asyncFall0().write(mask | event.fall0().read()) ;
    event.status0().write(mask) ;
    
    // precondition: pin must have been pulled to High by resistor!
    
    auto t0 = timer.cLo() ;
    // create Low pulse on pin thru GPIO mode switch
    function.set(pin,Rpi::Gpio::Function::Mode::Out) ;
    function.set(pin,Rpi::Gpio::Function::Mode::In) ;
    event.fall0().write(mask | event.fall0().read()) ;
    auto t1 = timer.cLo() ;

    if (0 == (event.status0().read() & mask))
    {
	std::cout << "error: no event detected or already cleared "
		  << "(" << (t1-t0) << "us)\n" ;
	return ;
    }

    while (0 != (event.status0().read() & mask))
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
