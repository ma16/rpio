// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "Host.h"
#include "Parser.h"
#include <Posix/base.h> // nanosleep
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>
#include <deque>
#include <fstream>
#include <iostream>

namespace Console { namespace Max7219 {

static void dataInvoke(Host *host,Ui::ArgL *argL)
{
  std::deque<uint16_t> q ;
  while (!argL->empty())
    q.push_back(Ui::strto<uint16_t>(argL->pop())) ;
  argL->finalize() ;
  for (auto &data: q)
    host->send(data) ;
  host->latch() ;
}
    
static void fileInvoke(Host *host,Ui::ArgL *argL)
{
  std::ifstream is(argL->pop().c_str()) ;
  auto n = Ui::strto<unsigned>(argL->option("-r","1")) ;
  argL->finalize() ;
  auto p = Parser(&is) ; std::deque<Console::Max7219::Parser::Command::shared_ptr> q ;
  while (true) {
    auto c = p.parse() ;
    auto eof = dynamic_cast<Console::Max7219::Parser::Eof*>(c.get()) ;
    if (eof != nullptr)
      break ;
    q.push_back(c) ;
  }
  for (auto i=0u ; i<n ; ++i) {
    for (auto &c : q) {
      auto delay = dynamic_cast<Console::Max7219::Parser::Delay*>(c.get()) ;
      if (delay != nullptr) { Posix::nanosleep(1E9*delay->seconds) ; continue ; }
      auto echo = dynamic_cast<Console::Max7219::Parser::Echo*>(c.get()) ;
      if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
      auto latch = dynamic_cast<Console::Max7219::Parser::Latch*>(c.get()) ;
      if (latch != nullptr) { host->latch() ; continue ; }
      auto shift = dynamic_cast<Console::Max7219::Parser::Shift*>(c.get()) ;
      if (shift != nullptr) { host->send(shift->data) ; continue ; }
      assert(false) ;
    }
  }
}
    
static void stdinInvoke(Host *host,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto p = Parser(&std::cin) ;
  while (true) {
    auto c = p.parse() ;
    auto delay = dynamic_cast<Console::Max7219::Parser::Delay*>(c.get()) ;
    if (delay != nullptr) { Posix::nanosleep(1E9*delay->seconds) ; continue ; }
    auto echo = dynamic_cast<Console::Max7219::Parser::Echo*>(c.get()) ;
    if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
    auto eof = dynamic_cast<Console::Max7219::Parser::Eof*>(c.get()) ;
    if (eof != nullptr) break ;
    auto latch = dynamic_cast<Console::Max7219::Parser::Latch*>(c.get()) ;
    if (latch != nullptr) { host->latch() ; continue ; }
    auto shift = dynamic_cast<Console::Max7219::Parser::Shift*>(c.get()) ;
    if (shift != nullptr) { host->send(shift->data) ; continue ; }
    assert(false) ;
  }
}
    
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: LOAD CLK DIN MODE\n"
	      << '\n'
	      << "LOAD : Pi's pin number connected to MAX7219 LOAD pin\n"
	      << " CLK : Pi's pin number connected to MAX7219  CLK pin\n"
	      << " DIN : Pi's pin number connected to MAX7219  DIN pin\n"
	      << '\n'
	      << "MODE : -d DATA+ | -i | -f FILE [-r N]\n"
	      << '\n'
	      << " -d : shift DATA+ and latch\n"
	      << " -f : read COMMAND+ until eof from FILE and execute thereafter\n"
	      << " -i : read COMMAND from standard-input and execute (repeat until eof)\n"
	      << " -r : execute N times (default 1)\n"
	      << '\n'
	      << "COMMAND+ : COMMAND | COMMAND COMMAND+\n"
	      << "   DATA+ :    DATA |    DATA    DATA+\n"
	      << '\n'
	      << " COMMAND : > DATA     # shift\n"
	      << "         | + SECONDS  # delay\n"
	      << "         | !          # latch (i.e. load)\n"
	      << "         | \" TEXT     # echo on standard-output\n"
	      << '\n'
	      << "    DATA : 0..65535\n"
	      << " SECONDS : floating point decimal number\n"
	      << "    TEXT : string terminated by double quotes (\")\n"
	      << std::flush ;
    return ;
  }
  
  auto loadPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto datPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

  Host host(Rpi::Gpio(rpi),loadPin,clkPin,datPin) ;
  
  auto arg = argL->pop() ;
  if      (arg == "-d") dataInvoke (&host,argL) ;
  else if (arg == "-i") stdinInvoke(&host,argL) ;
  else if (arg == "-f") fileInvoke (&host,argL) ;

  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Max7219 */ } /* Console */
