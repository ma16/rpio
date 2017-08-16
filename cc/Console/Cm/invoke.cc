// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Cm.h>
#include <Rpi/Gpio.h>
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

// --------------------------------------------------------------------

static void config(Rpi::Peripheral *rpi,Ui::ArgL *argL)
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

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
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
	std::cout << std::left
		  << std::setw(5) << getName(i.e())
		  << std::right
		  << std::setw(3) << cm.enabled(i.e()) 
		  << std::setw(4) << cm.   busy(i.e()) 
		  << std::setw(4) << cm. source(i.e()).count()
		  << std::setw(6) << cm.  intgr(i.e()).count()
		  << std::setw(6) << cm.  fract(i.e()).count()
		  << std::setw(5) << cm.   mash(i.e()).count() 
		  << '\n' ;
    }
    while (i.next()) ;
}

// --------------------------------------------------------------------

static void switch_(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: ALIAS MODE\n"
		  << "MODE: kill | off | on | toggle\n"
		  << std::flush ;
	return ;
    }
    auto alias = getAlias(argL) ;
    auto mode = argL->pop({"kill","off","on","toggle"}) ;
    argL->finalize() ;
    Rpi::Cm cm(rpi) ;
    if (mode == 3) 
	mode = cm.enabled(alias) ? 1 : 2 ;
    switch (mode) {
    case 0: cm.   kill(alias) ; break ;
    case 1: cm.disable(alias) ; break ;
    case 2: cm. enable(alias) ; break ;
    default: assert(false) ; abort() ;
    }
}

// --------------------------------------------------------------------

void Console::Cm::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE : config  # configure clock generator\n"
		  << "     | status  # display the status of all clock generators\n"
		  << "     | switch  # switch clock generator on/off\n"
		  << '\n'
		  << "There are seven clock generators specified by ALIAS:\n"
		  << "ALIAS: gp0 | gp1 | gp2 | pcm | pwm | slim | uart\n"
		  << std::flush ;
	return ;
    }
    
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map = {
	{ "config", config  },
	{ "status", status  },
	{ "switch", switch_ },
    } ;
    argL->pop(map)(rpi,argL) ;
}
