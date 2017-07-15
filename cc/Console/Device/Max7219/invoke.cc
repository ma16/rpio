// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include "Parser.h"
#include <Device/Max7219/Bang.h>
#include <Posix/base.h> // nanosleep
#include <Ui/strto.h>
#include <deque>
#include <fstream>
#include <iostream>

static void dataInvoke(Rpi::Peripheral *rpi,Device::Max7219::Bang *host,Ui::ArgL *argL)
{
    RpiExt::Bang::Enqueue q ;
    while (!argL->empty())
	host->send(&q,Ui::strto<uint16_t>(argL->pop())) ;
    host->load(&q) ;
    argL->finalize() ;
    
    RpiExt::Bang(rpi).execute(q.vector()) ;
}
    
static void fileInvoke(Rpi::Peripheral *rpi,Device::Max7219::Bang *host,Ui::ArgL *argL)
{
    std::ifstream is(argL->pop().c_str()) ;
    auto n = Ui::strto<unsigned>(argL->option("-r","1")) ;
    argL->finalize() ;
    
    using Parser = Console::Device::Max7219::Parser ;
    auto p = Parser(&is) ;
    std::deque<Parser::Command::shared_ptr> q ;
    
    while (true)
    {
	auto c = p.parse() ;
	auto eof = dynamic_cast<Parser::Eof*>(c.get()) ;
	if (eof != nullptr)
	    break ;
	q.push_back(c) ;
    }
    
    for (auto i=0u ; i<n ; ++i)
    {
	RpiExt::Bang::Enqueue data ;
    
	for (auto &c : q)
	{
	    auto delay = dynamic_cast<Parser::Delay*>(c.get()) ;
	    if (delay != nullptr) { Posix::nanosleep(1E9*delay->seconds) ; continue ; }
	    
	    auto echo = dynamic_cast<Parser::Echo*>(c.get()) ;
	    if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
	    
	    auto latch = dynamic_cast<Parser::Latch*>(c.get()) ;
	    if (latch != nullptr)
	    {
		host->load(&data) ;
		RpiExt::Bang(rpi).execute(data.vector()) ;
		data = RpiExt::Bang::Enqueue() ;
		continue ;
	    }
	
	    auto shift = dynamic_cast<Parser::Shift*>(c.get()) ;
	    if (shift != nullptr) { host->send(&data,shift->data) ; continue ; }
	    
	    assert(false) ;
	}
    }
}
    
static void stdinInvoke(Rpi::Peripheral *rpi,Device::Max7219::Bang *host,Ui::ArgL *argL)
{
    argL->finalize() ;

    using Parser = Console::Device::Max7219::Parser ;
    auto p = Parser(&std::cin) ;
    
    RpiExt::Bang::Enqueue q ;
    
    while (true)
    {
	auto c = p.parse() ;
	
	auto delay = dynamic_cast<Parser::Delay*>(c.get()) ;
	if (delay != nullptr) { Posix::nanosleep(1E9*delay->seconds) ; continue ; }
	
	auto echo = dynamic_cast<Parser::Echo*>(c.get()) ;
	if (echo != nullptr) { std::cout << echo->text << std::flush ; continue ; }
	
	auto eof = dynamic_cast<Parser::Eof*>(c.get()) ;
	if (eof != nullptr) break ;
	
	auto latch = dynamic_cast<Parser::Latch*>(c.get()) ;
	if (latch != nullptr)
	{
	    host->load(&q) ;
	    RpiExt::Bang(rpi).execute(q.vector()) ;
	    q = RpiExt::Bang::Enqueue() ;
	    continue ;
	}
	
	auto shift = dynamic_cast<Parser::Shift*>(c.get()) ;
	if (shift != nullptr) { host->send(&q,shift->data) ; continue ; }
	
	assert(false) ;
    }
}

void Console::Device::Max7219::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: DIN LOAD CLK MODE\n"
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

  auto  datPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto loadPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto  clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

  ::Device::Max7219::Bang host(rpi,datPin,loadPin,clkPin) ;
  
  auto arg = argL->pop() ;
  if      (arg == "-d") dataInvoke (rpi,&host,argL) ;
  else if (arg == "-i") stdinInvoke(rpi,&host,argL) ;
  else if (arg == "-f") fileInvoke (rpi,&host,argL) ;

  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
