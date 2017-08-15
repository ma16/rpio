// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Cm.h>
#include <Rpi/Gpio.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

Rpi::Cm::Alias getAlias(Ui::ArgL *argL)
{
    auto index = argL->pop({"gp0","gp1","gp2","pwm"}) ;
    switch (index)
    {
    case 0: return Rpi::Cm::Alias::Gp0 ;
    case 1: return Rpi::Cm::Alias::Gp1 ;
    case 2: return Rpi::Cm::Alias::Gp2 ;
    case 3: return Rpi::Cm::Alias::Pwm ;
    default: assert(false) ;
    }
    abort() ;
}

// --------------------------------------------------------------------

static void setInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: ALIAS [-f FRACT] [-i INT] [-m MASH] [-s SOURCE]\n"
	    << '\n'
	    << " FRACT : fractional part of the pre-scaler (0..FFF)\n"
	    << "   INT :    integer part of the pre-scaler (0..FFF)\n"
	    << "  MASH : apply 'mash' filter (0..3, where 0 means off)\n"
	    << '\n'
	    << "SOURCE : the clock source (0..15); for example:\n"
	    << "           1 = ocillator =    19.2 Mhz\n"
	    << "           6 = PLLD      =   500.0 MHz\n"
	    ;
	return ;
    }
    
    auto alias = getAlias(argL) ;
    Rpi::Cm cm(rpi) ;

    auto fract = cm.fract(alias) ;
    if (argL->pop_if("-f"))
	fract = Ui::strto(argL->pop(),Rpi::Cm::Div::Fract::Uint()) ;

    auto intgr = cm.intgr(alias) ;
    if (argL->pop_if("-i"))
	intgr = Ui::strto(argL->pop(),Rpi::Cm::Div::Intgr::Uint()) ;
  
    auto mash = cm.mash(alias) ;
    if (argL->pop_if("-m"))
	mash = Ui::strto(argL->pop(),Rpi::Cm::Ctl::Mash::Uint()) ;
  
    auto source = cm.source(alias) ;
    if (argL->pop_if("-s"))
	source = Ui::strto(argL->pop(),Rpi::Cm::Ctl::Src::Uint()) ;
  
    argL->finalize() ;
    cm.set(alias,source,intgr,fract,mash) ;
}

// --------------------------------------------------------------------

static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    { 
	std::cout << "There are no arguments\n" ;
	return ;
    }
    argL->finalize() ;
    Rpi::Cm cm(rpi) ; Rpi::Gpio gpio(rpi) ;

    std::cout << "alias on bsy src div:i div:f mash" << std::endl
	      << "---------------------------------" << std::endl ;

    auto i = Rpi::Cm::AliasN::first() ;
    do
    {
	static char const *name[] = { "gp0","gp1","gp2","pwm" } ;
	std::cout << name[i.n()] << "  "
		  << std::setw(3) << cm.enabled(i.e()) 
		  << std::setw(4) << cm.   busy(i.e()) 
		  << std::setw(4) << cm. source(i.e()).value()
		  << std::setw(6) << cm.  intgr(i.e()).value()
		  << std::setw(6) << cm.  fract(i.e()).value()
		  << std::setw(5) << cm.   mash(i.e()).value() 
		  << '\n' ;
    }
    while (i.next()) ;
}

// --------------------------------------------------------------------

static void switchInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: ALIAS MODE\n"
		  << '\n'
		  << "MODE: off | on | toggle | kill\n"
		  << std::flush ;
	return ;
    }
    auto alias = getAlias(argL) ;
    auto mode = argL->pop({"off","on","toggle","kill"}) ;
    argL->finalize() ;
    Rpi::Cm cm(rpi) ;
    if (mode == 2) 
	mode = !cm.enabled(alias) ;
    switch (mode) {
    case 0: cm.disable(alias) ; break ;
    case 1: cm. enable(alias) ; break ;
    case 3: cm.   kill(alias) ; break ;
    default: assert(false) ;
    }
}

// --------------------------------------------------------------------

void Console::Cm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : set     # set up clock parameters\n"
		  << "     | status  # display the status of all clock generators\n"
		  << "     | switch  # switch clock generator on/off\n"
		  << '\n'
		  << "There are four clock generators specified by ALIAS:\n"
		  << "ALIAS: gp0 | gp1 | gp2 | pwm\n"
		  << std::flush ;
	return ;
    }
    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "set"   )    setInvoke(rpi,argL) ; 
    else if (arg == "status") statusInvoke(rpi,argL) ; 
    else if (arg == "switch") switchInvoke(rpi,argL) ; 

    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
