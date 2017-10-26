// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/Cm.h>
#include <Rpi/GpioOld.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

static Rpi::Cm::Alias getAlias(Ui::ArgL *argL)
{
    auto index = argL->pop({"gp0","gp1","gp2","pcm","pwm","slim","uart"}) ;
    switch (index)
    {
    case 0: return Rpi::Cm::Alias::Gp0 ;
    case 1: return Rpi::Cm::Alias::Gp1 ;
    case 2: return Rpi::Cm::Alias::Gp2 ;
    case 3: return Rpi::Cm::Alias::Pcm ;
    case 4: return Rpi::Cm::Alias::Pwm ;
    case 5: return Rpi::Cm::Alias::Slim ;
    case 6: return Rpi::Cm::Alias::Uart ;
    default: assert(false) ;
    }
    abort() ;
}

static std::string getName(Rpi::Cm::Alias alias)
{
    switch (alias)
    {
    case Rpi::Cm::Alias::Gp0:  return  "gp0" ;
    case Rpi::Cm::Alias::Gp1:  return  "gp1" ;
    case Rpi::Cm::Alias::Gp2:  return  "gp2" ;
    case Rpi::Cm::Alias::Pcm:  return  "pcm" ;
    case Rpi::Cm::Alias::Pwm:  return  "pwm" ;
    case Rpi::Cm::Alias::Slim: return "slim" ;
    case Rpi::Cm::Alias::Uart: return "uart" ;
    default: assert(false) ;
    }
    abort() ;
}

static void ctl(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: ALIAS OPTION+\n"
	    << '\n'
	    << " [-+]enab  : enable the clock\n"
	    << " [-+]flip  : invert output signal\n"
	    << " [-+]kill  : reset clock\n"
	    << " mash 0..3 : audio-band noise-shaping\n"
	    << " src 0..15 : the clock source; for example:\n"
	    << "             1 = ocillator @  19.2 Mhz\n"
	    << "             6 = PLLD      @ 500.0 MHz\n"
	    << '\n'
	    << "Each option writes consecutively to the register\n"
	    ;
	return ;
    }
    
    auto alias = getAlias(argL) ;
    auto w = Rpi::Cm(rpi).ctl(alias).read() ;
    using Ctl = Rpi::Cm::Ctl ;
    
    while (!argL->empty())
    {
	auto opt = argL->pop({
		"-enab","+enab",
		"-flip","+flip",
		"-kill","+kill",
		"mash","src"}) ;
	switch (opt)
	{
	case 0: w -= Ctl::Enab::Digit ; break ;
	case 1: w += Ctl::Enab::Digit ; break ;
	    
	case 2: w -= Ctl::Flip::Digit ; break ;
	case 3: w += Ctl::Flip::Digit ; break ;
	    
	case 4: w -= Ctl::Kill::Digit ; break ;
	case 5: w += Ctl::Kill::Digit ; break ;
	    
	case 6: {
	    auto uint = Ui::strto(argL->pop(),Ctl::Mash::Uint()) ;
	    w %= Ctl::Mash(uint) ; break ;
	}
	case 7: {
	    auto uint = Ui::strto(argL->pop(),Ctl::Src::Uint()) ;
	    w %= Ctl::Src(uint) ; break ;
	}
	default: assert(false) ; abort() ;
	}
	Rpi::Cm(rpi).ctl(alias).write(w) ;
    }
    argL->finalize() ;
}

static void div(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: ALIAS OPTION+\n"
	    << '\n'
	    << "fract 0..4095 : fractional part of the divider\n"
	    << "intgr 0..4095 : integer part of the divider\n"
	    ;
	return ;
    }
    
    auto alias = getAlias(argL) ;
    auto w = Rpi::Cm(rpi).div(alias).read() ;
    using Div = Rpi::Cm::Div ;
    while (!argL->empty())
    {
	auto opt = argL->pop({"fract","intgr"}) ;
	switch (opt)
	{
	case 0: {
	    auto uint = Ui::strto(argL->pop(),Div::Fract::Uint()) ;
	    w %= Div::Fract(uint) ; break ;
	}
	case 1: {
	    auto uint = Ui::strto(argL->pop(),Div::Intgr::Uint()) ;
	    w %= Div::Intgr(uint) ; break ;
	}
	default: assert(false) ; abort() ;
	}
    }
    argL->finalize() ;
    Rpi::Cm(rpi).div(alias).write(w) ;
}

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    { 
	std::cout << "There are no arguments\n" ;
	return ;
    }
    argL->finalize() ;
    Rpi::Cm cm(rpi) ; Rpi::GpioOld gpio(rpi) ;

    std::cout << "alias on busy flip kill src div:i div:f mash\n" 
	      << "--------------------------------------------\n" ;

    auto i = Rpi::Cm::AliasN::first() ;
    do
    {
	auto ctl = cm.ctl(i.e()).read() ;
	auto div = cm.div(i.e()).read() ;
	using Ctl = Rpi::Cm::Ctl ;
	using Div = Rpi::Cm::Div ;
	std::cout << std::left
		  << std::setw(5) << getName(i.e())
		  << std::right
		  << std::setw(3) << ctl.test(Ctl::Enab::Digit) 
		  << std::setw(5) << ctl.test(Ctl::Busy::Digit)
		  << std::setw(5) << ctl.test(Ctl::Flip::Digit)
		  << std::setw(5) << ctl.test(Ctl::Kill::Digit) 
		  << std::setw(4) << ctl.test(Ctl::  Src()).count() 
		  << std::setw(6) << div.test(Div::Intgr()).count()
		  << std::setw(6) << div.test(Div::Fract()).count()
		  << std::setw(5) << ctl.test(Ctl:: Mash()).count()
		  << '\n' ;
    }
    while (i.next()) ;
}

void Console::Peripheral::Cm::
invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : ctl     # configure Control register\n"
		  << "     | div     # configure Divider register\n"
		  << "     | status  # display the status of all clock generators\n"
		  << '\n'
		  << "There are seven clock generators specified by ALIAS:\n"
		  << "ALIAS: gp0 | gp1 | gp2 | pcm | pwm | slim | uart\n"
		  << std::flush ;
	return ;
    }
    
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map = {
	{ "ctl"   ,    ctl },
	{ "div"   ,    div },
	{ "status", status },
    } ;
    argL->pop(map)(rpi,argL) ;
}
