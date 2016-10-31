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
#include "Host.h"
#include "Parser.h"
#include <Posix/base.h> // nanosleep
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>
#include <deque>
#include <fstream>
#include <iostream>

namespace Main { namespace Max7219 {

static void invokeCommand(Host *host,Ui::ArgL *argL)
{
  std::deque<uint16_t> q ;
  while (!argL->empty())
    q.push_back(Ui::strto<uint16_t>(argL->pop())) ;
  argL->finalize() ;
  for (auto &data: q)
    host->send(data) ;
  host->latch() ;
}
    
static void invokeScript(Host *host,Ui::ArgL *argL)
{
  std::ifstream is(argL->pop().c_str()) ;
  auto n = Ui::strto<unsigned>(argL->option("-r","1")) ;
  argL->finalize() ;
  auto p = Parser(&is) ; std::deque<Main::Max7219::Parser::Command::Ptr> q ;
  while (true) {
    auto c = p.parse() ;
    auto eof = dynamic_cast<Main::Max7219::Parser::Eof*>(c.get()) ;
    if (eof != nullptr)
      break ;
    q.push_back(std::move(c)) ;
  }
  for (auto i=0u ; i<n ; ++i) {
    for (auto &c : q) {
      auto delay = dynamic_cast<Main::Max7219::Parser::Delay*>(c.get()) ;
      if (delay != nullptr) { Posix::nanosleep(delay->seconds) ; continue ; }
      auto echo = dynamic_cast<Main::Max7219::Parser::Echo*>(c.get()) ;
      if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
      auto latch = dynamic_cast<Main::Max7219::Parser::Latch*>(c.get()) ;
      if (latch != nullptr) { host->latch() ; continue ; }
      auto shift = dynamic_cast<Main::Max7219::Parser::Shift*>(c.get()) ;
      if (shift != nullptr) { host->send(shift->data) ; continue ; }
      assert(false) ;
    }
  }
}
    
static void invokeStdin(Host *host,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto p = Parser(&std::cin) ;
  while (true) {
    auto c = p.parse() ;
    auto delay = dynamic_cast<Main::Max7219::Parser::Delay*>(c.get()) ;
    if (delay != nullptr) { Posix::nanosleep(delay->seconds) ; continue ; }
    auto echo = dynamic_cast<Main::Max7219::Parser::Echo*>(c.get()) ;
    if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
    auto eof = dynamic_cast<Main::Max7219::Parser::Eof*>(c.get()) ;
    if (eof != nullptr) break ;
    auto latch = dynamic_cast<Main::Max7219::Parser::Latch*>(c.get()) ;
    if (latch != nullptr) { host->latch() ; continue ; }
    auto shift = dynamic_cast<Main::Max7219::Parser::Shift*>(c.get()) ;
    if (shift != nullptr) { host->send(shift->data) ; continue ; }
    assert(false) ;
  }
}
    
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: {loadPin} {clkPin} {dinPin} {mode}\n"
	      << "\n"
	      << "{loadPin} : Pi's pin for MAX7219 LOAD pin\n"
	      << " {clkPin} : Pi's pin for MAX7219  CLK pin\n"
	      << " {dinPin} : Pi's pin for MAX7219  DIN pin\n"
	      << "\n"
	      << "{mode} : -d {data}*\n"
	      << "       | -i\n"
	      << "       | -f {file} [-r {n}]\n"
	      << "\n"
	      << " -d {data}* : shift {data} and latch\n"
	      << " -i         : {command} sequence is read from stdin and immediately executed\n"
	      << " -f {file}  : {command} sequence is read from file; thereafter executed\n"
	      << " -r {n}     : repeat {n} times\n"
	      << "\n"
	      << "{command} : '>' {data}    // shift data\n"
	      << "          | '+' {seconds} // delay seconds\n"
	      << "          | '!'           // latch (load)\n"
	      << "          | '\"' {text}    // echo text on console\n"
	      << "\n"
	      << "   {data} : unsigned 16-bit integer\n"
	      << "{seconds} : floating point decimal\n"
	      << "   {text} : text terminated by double quotes (\")\n"
	      << std::flush ;
    return ;
  }
  
  auto loadPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto datPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

  Host host(rpi->gpio().get(),loadPin,clkPin,datPin) ;
  
  auto arg = argL->pop() ;
  if      (arg == "-d") invokeCommand(&host,argL) ;
  else if (arg == "-i") invokeStdin (&host,argL) ;
  else if (arg == "-f") invokeScript (&host,argL) ;

  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Max7219 */ } /* Main */
