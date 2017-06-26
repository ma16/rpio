// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Cp.h>
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
  auto index = Ui::strto(argL->pop(),Rpi::Cp::Index()) ;
  Rpi::Cp cp(rpi) ;

  auto fract = cp.fract(index) ;
  if (argL->pop_if("-f"))
    fract = Ui::strto(argL->pop(),Rpi::Cp::Divider::Fract()) ;

  auto intgr = cp.intgr(index) ;
  if (argL->pop_if("-i"))
    intgr = Ui::strto(argL->pop(),Rpi::Cp::Divider::Intgr()) ;
  
  auto mash = cp.mash(index) ;
  if (argL->pop_if("-m"))
    mash = Ui::strto(argL->pop(),Rpi::Cp::Control::Mash()) ;
  
  auto source = cp.source(index) ;
  if (argL->pop_if("-s"))
    source = Ui::strto(argL->pop(),Rpi::Cp::Control::Source()) ;
  
  argL->finalize() ;
  cp.set(index,source,intgr,fract,mash) ;
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
  Rpi::Cp cp(rpi) ; Rpi::Gpio gpio(rpi) ;

  std::cout << "# on bsy src div:i div:f mash" << std::endl
	    << "-----------------------------" << std::endl ;

  auto i = Rpi::Cp::Index::first() ; do {
    std::cout << std::setw(1) << i.value() 
	      << std::setw(3) << cp.enabled(i) 
	      << std::setw(4) << cp.   busy(i) 
	      << std::setw(4) << cp. source(i).value()
	      << std::setw(6) << cp.  intgr(i).value()
	      << std::setw(6) << cp.  fract(i).value()
	      << std::setw(5) << cp.   mash(i).value() 
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
  auto index = Ui::strto(argL->pop(),Rpi::Cp::Index()) ;
  auto mode = argL->pop({"off","on","toggle","kill"}) ;
  argL->finalize() ;
  Rpi::Cp cp(rpi) ;
  if (mode == 2) 
    mode = !cp.enabled(index) ;
  switch (mode) {
  case 0: cp.disable(index) ; break ;
  case 1: cp. enable(index) ; break ;
  case 3: cp.   kill(index) ; break ;
  default: assert(false) ;
  }
}

// --------------------------------------------------------------------

void Console::Cp::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
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
