// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Counter.h>
#include <Ui/strto.h>
#include <iostream>
#include <thread> // sleep_for, chrono

static void set(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: MODE | DIV | MODE DIV\n" 
	    << '\n'
	    << "MODE | off  # disable ARM counter\n"
	    << "     | on   #  enable ARM counter\n"
	    << '\n'
	    << "DIV = pre-scaler (0..255)\n"
	    ;
	// ...[todo] verify DIV range agains actual DIV type
	return ;
    }
    auto enable = argL->pop_if({"off","on"}) ;
    boost::optional<Rpi::Counter::Div> div ;
    if (!argL->empty())
	div = Ui::strto<Rpi::Counter::Div>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Counter counter(rpi) ;
    if (div) 
	counter.prescale(*div) ;
    if (enable)
	counter.enable(*enable != 0) ;
}

static void sleep(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "argument: [-b] TIME\n"
	    << '\n'
	    << "  -b: do busy-wait instead of sleeping\n"
	    << "TIME: time to sleep in (fractions of) seconds\n" ;
	return ;
    }
    auto busy = argL->pop_if("-b") ;
    auto seconds = Ui::strto<double>(argL->pop()) ;
    // ...[todo] this should only accept unsigned values
    argL->finalize() ;
  
    Rpi::Counter counter(rpi) ;
    auto c0 = counter.clock() ;
    auto s0 = std::chrono::steady_clock::now() ;

    if (busy)
    {
      Loop:
	auto ds = std::chrono::steady_clock::now() - s0 ;
	auto elapsed = std::chrono::duration<double>(ds).count() ;
	if (elapsed < seconds)
	    goto Loop ;
    }
    else
    {
	auto ns = static_cast<uint64_t>(1e+9 * seconds + .5) ;
	// ...[todo] watch overflows
	std::this_thread::sleep_for(std::chrono::nanoseconds(ns)) ;
    }
    
    auto dc = counter.clock() - c0 ;
    // ...[todo] the counter might have wrapped around
    auto ds = std::chrono::steady_clock::now() - s0 ;
    // ...this value can be misleading
    // -- if sleep time was quite short or
    // -- if we got suspended immediately before this call
    auto elapsed = std::chrono::duration<double>(ds).count() ;
    
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(3) ;
    std::cout
	<< "ticks="   << dc         << ' '
	<< "elapsed=" << elapsed    << "s "
	<< "ticks/s=" << dc/elapsed << '\n' ;
}
    
static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    { 
	std::cout << "no arguments\n" ;
	return ;
    }
    argL->finalize() ;
    Rpi::Counter counter(rpi) ;
    std::cout
	<< "enabled....." << (counter.enabled() ? "on" : "off") << '\n'
	<< "prescaler..." << std::to_string(counter.prescaler()) << '\n'
	<< "ticks......." << counter.clock() << '\n'
	;
}

void Console::Counter::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: MODE [help]\n"
	    << '\n'
	    << "MODE : set     # switch on/off and set prescaler\n"
	    << "     | sleep   # show counter rate for a given duration\n"
	    << "     | status  # display status\n"
	    ;
	return ;
    }
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "set"    ,    set },
	{ "sleep"  ,  sleep },
	{ "status" , status },
    } ;
    argL->pop(map)(rpi,argL) ;
}
