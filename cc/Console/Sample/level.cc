// BSD 2-Clause License, see github.com/ma16/rpio

// see README.md for details

#include "invoke.h"
#include <Rpi/Pin.h>
#include <Rpi/Register.h>
#include <Ui/strto.h>
#include <chrono>
#include <iomanip>
#include <iostream>

using Clock = std::chrono::steady_clock ;

using Duration = std::chrono::duration<double> ;

namespace Register = Rpi::Register::Gpio ;

// --------------------------------------------------------------------

// sample input level register
static void frequency(Rpi::Peripheral *rpi,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto nsamples = Ui::strto<uint32_t>(argL->pop()) ;
    auto dry = argL->pop_if("-d") ;
    argL->finalize() ;
    auto input = rpi->at<Register::Input::Bank0>() ;
    auto mask = 1u << pin.value() ;
    decltype(nsamples) nchanges = 0 ; // transition counter
    decltype(nsamples) nhi = 0 ; // High-level counter
    auto level = input.read().test(mask) ;
    auto t0 = Clock::now() ;
    if (dry)
    {
	// this dry-run might be useful to estimate the additional
	// cpu-time that is required to execute the real test.
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    level ^= input.read().test(mask) ;
	    // ...level is used to prevent compiler optimizations
	}
	auto volatile x = level ; (void)x ;
    }
    else
    {
	for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
	{
	    auto next = input.read().test(mask) ;
	    if (next != level)
	    {
		level = next ;
		++nchanges ;
	    }
	    if (next)
		++nhi ;
	}
    }
    auto t1 = Clock::now() ; 
    auto dt = Duration(t1-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    
    std::cout << "r=" << nsamples/dt << "/s " 
	      << "f=" << nchanges/dt/2 << "/s "
	      << "duty=" << 1.0 * nhi / nsamples << "\n" ;
}

// --------------------------------------------------------------------

void Console::Sample::invoke_level(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: MODE PIN NSAMPLES\n"
	    << '\n'
	    << "frequency: displays the sampling rate (f) and the signal rate" 
	    << '\n'
	    << "-d : dry run to determine maximum sampling rate f\n"
	    ;
	return ;
    }

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "frequency" , frequency },
    } ;
    argL->pop(map)(rpi,argL) ;
    
}
