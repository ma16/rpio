// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Cm.h>
#include <Rpi/Gpio.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

// --------------------------------------------------------------------

static void setInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: INDEX [-f FRACT] [-i INT] [-m MASH] [-s SOURCE] \n"
	      << '\n'
	      << " FRACT : fractional part of the pre-scaler (0..FFF)\n"
	      << "   INT :    integer part of the pre-scaler (0..FFF)\n"
	      << "  MASH : apply 'mash' filter (0..3, where 0 means off)\n"
	      << '\n'
	      << "SOURCE : the clock source (0..15); for example:\n"
	      << "           1 = ocillator =    19.2 Mhz\n"
	      << "           6 = PLLD      =   500.0 MHz\n"
	      << std::flush ;
    return ;
  }
  auto index = Ui::strto(argL->pop(),Rpi::Cm::Index()) ;
  Rpi::Cm cm(rpi) ;

  auto fract = cm.fract(index) ;
  if (argL->pop_if("-f"))
    fract = Ui::strto(argL->pop(),Rpi::Cm::Divider::Fract()) ;

  auto intgr = cm.intgr(index) ;
  if (argL->pop_if("-i"))
    intgr = Ui::strto(argL->pop(),Rpi::Cm::Divider::Intgr()) ;
  
  auto mash = cm.mash(index) ;
  if (argL->pop_if("-m"))
    mash = Ui::strto(argL->pop(),Rpi::Cm::Control::Mash()) ;
  
  auto source = cm.source(index) ;
  if (argL->pop_if("-s"))
    source = Ui::strto(argL->pop(),Rpi::Cm::Control::Source()) ;
  
  argL->finalize() ;
  cm.set(index,source,intgr,fract,mash) ;
}

// --------------------------------------------------------------------

static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") { 
    std::cout << "There are no arguments\n"
	      << std::flush ;
    return ;
  }
  argL->finalize() ;
  Rpi::Cm cm(rpi) ; Rpi::Gpio gpio(rpi) ;

  std::cout << "# on bsy src div:i div:f mash" << std::endl
	    << "-----------------------------" << std::endl ;

  auto i = Rpi::Cm::Index::first() ; do {
    std::cout << std::setw(1) << i.value() 
	      << std::setw(3) << cm.enabled(i) 
	      << std::setw(4) << cm.   busy(i) 
	      << std::setw(4) << cm. source(i).value()
	      << std::setw(6) << cm.  intgr(i).value()
	      << std::setw(6) << cm.  fract(i).value()
	      << std::setw(5) << cm.   mash(i).value() 
	      << '\n' ;
  }
  while (i.next()) ;
}

// --------------------------------------------------------------------

static void switchInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: INDEX MODE\n"
	      << '\n'
	      << "MODE: off | on | toggle | kill\n"
	      << std::flush ;
    return ;
  }
  auto index = Ui::strto(argL->pop(),Rpi::Cm::Index()) ;
  auto mode = argL->pop({"off","on","toggle","kill"}) ;
  argL->finalize() ;
  Rpi::Cm cm(rpi) ;
  if (mode == 2) 
    mode = !cm.enabled(index) ;
  switch (mode) {
  case 0: cm.disable(index) ; break ;
  case 1: cm. enable(index) ; break ;
  case 3: cm.   kill(index) ; break ;
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
	      << "There are three clock generators specified by INDEX:\n"
	      << "INDEX: 0 | 1 | 2\n"
	      << std::flush ;
    return ;
  }
  std::string arg = argL->pop() ;
  if      (arg == "set"   )    setInvoke(rpi,argL) ; 
  else if (arg == "status") statusInvoke(rpi,argL) ; 
  else if (arg == "switch") switchInvoke(rpi,argL) ; 
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
