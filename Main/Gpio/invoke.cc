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
    std::cout << "arguments: {pins} i|o|0|1|2|3|4|5\n"
	      << "\n"
	      << "   {pins}: <no> | -l <no>(,<no>)* | -m <mask> | all\n"
	      << std::flush ;
    return ; 
  }
  auto pins = getPins(argL) ;

  auto i = argL->pop({"i","o","0","1","2","3","4","5"}) ;
  Rpi::Gpio::Mode mode = Rpi::Gpio::Mode::Base() ;
  switch (i) {
  case 0: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::  In>() ; break ;
  case 1: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode:: Out>() ; break ;
  case 2: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt0>() ; break ;
  case 3: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt1>() ; break ;
  case 4: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt2>() ; break ;
  case 5: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt3>() ; break ;
  case 6: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt4>() ; break ;
  case 7: mode = Rpi::Gpio::Mode::init<Rpi::Gpio::Mode::Alt5>() ; break ;
  }
  
  argL->finalize() ;
  rpi->gpio()->setMode(pins,mode) ;
}

static void outputInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "argument: {pins} lo|hi\n"
	      << "\n"
	      << "   {pins}: <no> | -l <no>(,<no>)* | -m <mask> | all\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  auto i = argL->pop({"lo","hi"}) ;
  Rpi::Gpio::Output output = Rpi::Gpio::Output::Base() ;
  switch (i) {
  case 0: output = Rpi::Gpio::Output::init<Rpi::Gpio::Output::Lo>() ; break ;
  case 1: output = Rpi::Gpio::Output::init<Rpi::Gpio::Output::Hi>() ; break ;
  }
  argL->finalize() ;
  rpi->gpio()->setOutput(pins,output) ;
}

static void pullInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "argument: {pins} up|down|off\n"
	      << "\n"
	      << "   {pins}: <no> | -l <no>(,<no>)* | -m <mask> | all\n"
	      << std::flush ;
    return ;
  }
  auto pins = getPins(argL) ;
  
  auto i = argL->pop({"up","down","off"}) ;
  Rpi::Gpio::Pull pull = Rpi::Gpio::Pull::Base() ;
  switch (i) {
  case 0: pull = Rpi::Gpio::Pull::init<Rpi::Gpio::Pull::  Up>() ; break ;
  case 1: pull = Rpi::Gpio::Pull::init<Rpi::Gpio::Pull::Down>() ; break ;
  case 2: pull = Rpi::Gpio::Pull::init<Rpi::Gpio::Pull:: Off>() ; break ;
  }
  
  argL->finalize() ;
  rpi->gpio()->setPull(pins,pull) ; 
}
    
static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (!argL->empty() && argL->peek() == "help") {
    std::cout << "arguments: [{pins}]\n"
	      << "\n"
	      << "   {pins}: <no> | -l <no>(,<no>)* | -m <mask> | all\n"
	      << std::flush ;
    return ;
  }
  auto pins = 0xffffffffu ;
  if (!argL->empty())
    pins = getPins(argL) ;
  argL->finalize() ;

  std::cout << mkhdr(pins) << std::endl ;
  std::cout << mksep(pins) << std::endl ;
  auto i = Rpi::Pin::first() ;
  do {
    if (0 == (pins & (1u << i.value())))
      continue ;
    switch (rpi->gpio()->getMode(i).value()) {
    case Rpi::Gpio::Mode::  In: std::cout << "i " ; break ;
    case Rpi::Gpio::Mode:: Out: std::cout << "o " ; break ;
    case Rpi::Gpio::Mode::Alt0: std::cout << "0 " ; break ;
    case Rpi::Gpio::Mode::Alt1: std::cout << "1 " ; break ;
    case Rpi::Gpio::Mode::Alt2: std::cout << "2 " ; break ;
    case Rpi::Gpio::Mode::Alt3: std::cout << "3 " ; break ;
    case Rpi::Gpio::Mode::Alt4: std::cout << "4 " ; break ;
    case Rpi::Gpio::Mode::Alt5: std::cout << "5 " ; break ;
    default: assert(false) ;
    }
  } while (i.next()) ;                  std::cout << "mode" << std::endl ;
  std::cout << mkstr(pins,rpi->gpio()->getLevels()) << "level" << std::endl ;
}

void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: (mode | output | pull | status) [help]\n" ;
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
