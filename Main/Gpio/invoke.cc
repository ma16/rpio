// Copyright (c) 2016, "ma16". All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// This project is hosted at https://github.com/ma16/rpio

#include "../rpio.h"
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>
#include <iostream>

namespace Main { namespace Gpio {

static uint32_t getPins(Ui::ArgL *argL)
{
  auto arg = argL->pop() ;
  if (arg == "all") 
    return 0xffffffff ;
  if (arg == "-m") 
    return Ui::strto<uint32_t>(argL->pop()) ;
  if (arg == "-l") {
    auto arg = argL->pop() ;
    auto tail = arg.find(',') ;
    decltype(tail) head = 0 ;
    auto mask = 0u ;
    while (tail != arg.npos) {
      auto pin = Ui::strto(arg.substr(head,tail-head),Rpi::Pin()) ;
      mask |=  (1u << pin.value()) ;
      head = tail + 1 ;
      tail = arg.find(',',head) ;
    }
    auto pin = Ui::strto(arg.substr(head),Rpi::Pin()) ;
    mask |=  (1u << pin.value()) ;
    return mask ;
  }
  auto pin = Ui::strto(arg,Rpi::Pin()) ;
  return (1u << pin.value()) ;
}

// --------------------------------------------------------------------
    
static std::string mkhdr(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i/10 << ' ' ;
  os << '\n' ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i%10 << ' ' ;
  return os.str() ;
}

static std::string mksep(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << "--" ;
  return os.str() ;
}

static std::string mkstr(uint32_t mask,uint32_t bits)
{
  char s[32*2] ;
  auto ofs = 0u ;
  for (uint32_t m=1u ; m!=0 ; m<<=1) {
    if (m & mask) {
      s[ofs++] = (bits & m) ? '1' : '0' ;
      s[ofs++] = ' ' ;
    }
  }  
  return std::string(s,ofs) ;
} 

// --------------------------------------------------------------------
    
static void modeInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : i  # as input\n"
	      << "     | o  # as output\n"
	      << "     | 0  # as alternate function 0\n"
	      << "     | 1  # as alternate function 1\n"
	      << "     | 2  # as alternate function 2\n"
	      << "     | 3  # as alternate function 3\n"
	      << "     | 4  # as alternate function 4\n"
	      << "     | 5  # as alternate function 5\n"
	      << std::flush ;
    return ; 
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"i","o","5","4","0","1","2","3"}) ;
  auto mode = Rpi::Gpio::ModeN::make(i).e() ;
  argL->finalize() ;
  rpi->gpio()->setMode(pins,mode) ;
}

static void outputInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : lo  # set output low\n"
	      << "     | hi  # set output high\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"hi","lo"}) ;
  auto output = Rpi::Gpio::OutputN::make(i).e() ;
  argL->finalize() ;
  rpi->gpio()->setOutput(pins,output) ;
}

static void pullInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: PINS MODE\n"
	      << '\n'
	      << "MODE : down  # apply pull-down resistor\n"
	      << "     | off   # set tri-state (high-impedance)\n"
	      << "     | up    # apply pull-up resistor\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"off","down","up"}) ;
  auto pull = Rpi::Gpio::PullN::make(i).e() ;
  argL->finalize() ;
  rpi->gpio()->setPull(pins,pull) ; 
}
    
static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "arguments: [PINS]\n"
	      << '\n'
	      << "Displays for all (default) or for selected pins:\n"
	      << "* mode  : i,o,0..5 (see mode's help)\n"
	      << "* level : the input level (0:low, 1:high)\n"
	      << std::flush ;
    return ;
  }
  auto pins = 0xffffffffu ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;

  std::cout << mkhdr(pins) << '\n'
	    << mksep(pins) << '\n' ;
  auto i = Rpi::Pin::first() ;
  do {
    if (0 == (pins & (1u << i.value())))
      continue ;
    static const char m[] = { 'i','o','5','4','0','1','2','3' } ;
    auto mode = rpi->gpio()->getMode(i) ;
    std::cout << m[Rpi::Gpio::ModeN(mode).n()] << ' ' ;
  } while (i.next()) ;
  std::cout << "mode\n" ;
  std::cout << mkstr(pins,rpi->gpio()->getLevels()) << "level" << std::endl ;
}

void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [help]\n"
	      << '\n'
	      << "MODE : mode    # set mode\n"
	      << "     | output  # set level\n"
	      << "     | pull    # switch resistor\n"
	      << "     | status  # display status\n"
	      << '\n'
	      << "Arguments may require a PINS value:\n"
	      << '\n'
	      << "PINS : NO             # a single pin number\n"
	      << "     | -l NO[,NO]...  # a set of pin numbers as list\n"
	      << "     | -m MASK        # a set of pin number as bit mask\n"
	      << "     | all            # all pins\n"
	      << std::flush ;
    return ;
  }

  std::string arg = argL->pop() ;

  if      (arg ==   "mode")   modeInvoke(rpi,argL) ;
  else if (arg == "output") outputInvoke(rpi,argL) ;
  else if (arg ==   "pull")   pullInvoke(rpi,argL) ;
  else if (arg == "status") statusInvoke(rpi,argL) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Gpio */ } /* Main */
