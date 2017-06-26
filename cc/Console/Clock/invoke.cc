// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Access various clock sources
// --------------------------------------------------------------------

#include "../rpio.h"
#include <chrono>
#include <Posix/base.h> // nanosleep
#include <Rpi/Counter.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>
#include <iostream>

namespace Console { namespace Clock {

void countInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE COUNT\n"
	      << '\n'
	      << "MODE : arm     # ARM Counter\n"
	      << "     | bcm     # BCM Counter\n"
	      << "     | chrono  # chrono::steady_clock\n"
	      << "     | sleep   # POSIX nanosleep\n"
	      << '\n'
	      << "If sleep then suspend process for COUNT nano seconds.\n"
	      << "Otherwise execute a busy loop until COUNT is reached.\n"
	      << "After that display the passed ticks for each clock.\n"
	      << std::flush ;
    
    return ;
  }
  auto mode = argL->pop({"arm","bcm","chrono","sleep"}) ;
  auto count = Ui::strto<uint64_t>(argL->pop()) ;
  argL->finalize() ;
  Rpi::Counter counter(rpi) ;
  Rpi::Timer timer(rpi) ;
  auto a0 = counter.clock() ;
  auto b0 = timer.clock() ;
  auto c0 = std::chrono::steady_clock::now().time_since_epoch().count() ;
  unsigned i = 0 ;
  switch (mode) {
  case 0: 
    while ((counter.clock()-a0) < count)
      ++i ;
    break ;
  case 1:
    while ((timer.clock()-b0) < count)
      ++i ;
    break ;
  case 2:
    while (Neat::as_unsigned(std::chrono::steady_clock::now().time_since_epoch().count()-c0) < count)
      ++i ;
    break ;
  case 3:
    Posix::nanosleep(static_cast<double>(count)) ;
    break ;
  default: assert(false) ;
  }
  auto ax = counter.clock() - a0 ;
  auto bx = timer.clock() - b0 ;
  auto cx = std::chrono::steady_clock::now().time_since_epoch().count() - c0 ;
  std::cout << "After " << i << " iterations:\n"
	    << "arm......" << ax << '\n' 
	    << "bcm......" << bx << '\n'
	    << "chrono..." << cx << '\n'
	    << std::flush ;
}
    
void setInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [DIV]\n" 
	      << '\n'
	      << "MODE : -    # leave ARM counter as is\n"
	      << "     | off  # disable ARM counter\n"
	      << "     | on   # enable ARM counter\n"
	      << '\n'
	      << "DIV is the pre-scaler (0..255) to be applied.\n"
	      << std::flush ;
    return ;
  }
  auto type = argL->pop({"off","on","-"}) ;
  boost::optional<Rpi::Counter::Div> div ;
  if (!argL->empty())
    div = Ui::strto<Rpi::Counter::Div>(argL->pop()) ;
  argL->finalize() ;
  Rpi::Counter counter(rpi) ;
  if (div) 
    counter.prescale(*div) ;
  switch(type) {
  case 0: counter.enable(false) ; break ;
  case 1: counter.enable( true) ; break ;
  case 2: break ;
  default: assert(false) ;
  }
}

void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") { 
    std::cout << "There are no arguments\n" ;
    return ;
  }
  argL->finalize() ;
  Rpi::Counter counter(rpi) ;
  Rpi::Timer timer(rpi) ;
  std::cout << "BCM System Timer Peripheral Counter (bcm):\n"
	    << "ticks......." << timer.clock() << '\n'
	    << '\n'
	    << "ARM Free Running Counter (arm):\n"
	    << "enabled....." << counter.enabled() << '\n'
	    << "prescaler..." << std::to_string(counter.prescaler()) << '\n'
	    << "ticks......." << counter.clock() << '\n'
	    << '\n'
	    << "C++ chrono::steady_clock (chrono):\n"
	    << "ticks......." << std::chrono::steady_clock::now().time_since_epoch().count() << '\n'
	    << std::flush ;
}

void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [help]\n"
	      << '\n'
	      << "MODE : count   # verify clock resolution\n"
      	      << "     | set     # configure ARM counter\n"
	      << "     | status  # display clock values\n"
	      << '\n'
	      << "The supported clock sources are:\n"
	      << "* arm    : ARM Free Running Counter (B00:420+4)\n"
	      << "* bcm    : BCM System Timer Peripheral Counter (003:004+8)\n"
	      << "* chrono : C++ chrono::steady_clock (clock_gettime(CLOCK_MONOTONIC))\n"
	      << '\n'
	      << "The clock resolutions are (seconds):\n"
	      << "* arm    : up to 2.5E-9\n"
	      << "* bcm    : 1E-6\n"
	      << "* chrono : 1E-9\n"
	      << std::flush ;
    return ;
  }
  std::string arg = argL->pop() ;
  if      (arg ==   "count")  countInvoke(rpi,argL) ;
  else if (arg ==     "set")    setInvoke(rpi,argL) ;
  else if (arg ==  "status") statusInvoke(rpi,argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Clock */ } /* Console */
