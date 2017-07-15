// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <deque>
#include <fstream>
#include <iostream>
#include <Device/Max7219/Bang.h>
#include <Device/Max7219/Parser.h>
#include <Posix/base.h> // nanosleep
#include <Ui/strto.h>

using namespace Device::Max7219 ;

static void
dataInvoke(Rpi::Peripheral *rpi,Bang *bang,Ui::ArgL *argL)
{
    RpiExt::Bang::Enqueue script ;
    while (!argL->empty())
	bang->send(&script,Ui::strto<uint16_t>(argL->pop())) ;
    bang->load(&script) ;
    argL->finalize() ;
    RpiExt::Bang(rpi).execute(script.vector()) ;
}
    
static void
fileInvoke(Rpi::Peripheral *rpi,Bang *bang,Ui::ArgL *argL)
{
    std::ifstream is(argL->pop().c_str()) ;
    auto n = Ui::strto<unsigned>(argL->option("-r","1")) ;
    argL->finalize() ;
    
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
	RpiExt::Bang::Enqueue script ;
    
	for (auto &c : q)
	{
	    auto delay = dynamic_cast<Parser::Delay*>(c.get()) ;
	    if (delay != nullptr)
	    { Posix::nanosleep(1E9*delay->seconds) ; continue ; }
	    
	    auto echo = dynamic_cast<Parser::Echo*>(c.get()) ;
	    if (echo != nullptr)
	    { std::cout << echo->text << std::flush ; continue ; }
	    
	    auto latch = dynamic_cast<Parser::Latch*>(c.get()) ;
	    if (latch != nullptr)
	    {
		bang->load(&script) ;
		RpiExt::Bang(rpi).execute(script.vector()) ;
		script = RpiExt::Bang::Enqueue() ;
		continue ;
	    }
	
	    auto shift = dynamic_cast<Parser::Shift*>(c.get()) ;
	    if (shift != nullptr)
	    { bang->send(&script,shift->data) ; continue ; }
	    
	    assert(false) ;
	}
    }
}
    
static Bang::Seconds getTiming(Ui::ArgL *argL)
{
    if (argL->pop_if("-t"))
    {
	Bang::Seconds s(
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop())) ;
	return s ;
    }

    return Bang::strict() ;
}

void Console::Device::Max7219::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: DIN LOAD CLK [-f FREQ] MODE\n"
		  << '\n'
		  << " DIN : Pi's pin number connected to MAX7219  DIN pin\n"
		  << "LOAD : Pi's pin number connected to MAX7219 LOAD pin\n"
		  << " CLK : Pi's pin number connected to MAX7219  CLK pin\n"
		  << '\n'
		  << "FREQ: ARM counter frequency that has been set up\n"
		  << '\n'
		  << "MODE : -d DATA+ | -f FILE [-r N]\n"
		  << '\n'
		  << " -d : shift DATA+ and latch\n"
		  << " -r : execute N times (default 1)\n"
		  << " -s : read COMMAND+ until eof from FILE and execute\n"
		  << '\n'
		  << "COMMAND+ : COMMAND | COMMAND COMMAND+\n"
		  << "   DATA+ :    DATA |    DATA    DATA+\n"
		  << '\n'
		  << " COMMAND : '>' DATA     # 16-bit word to serialize\n"
		  << "         | '+' SECONDS  # number of seconds to sleep\n"
		  << "         | '!'          # activate LOAD pin\n"
		  << "         | '\"' TEXT '\"' # print text to stdout\n" ;
	return ;
    }

    auto  datPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto loadPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto  clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    auto f = Rpi::Counter(rpi).frequency() ;
    if (argL->pop_if("-f"))
    {
	auto g = Ui::strto<double>(argL->pop()) ;
	if (g/f < 0.95 || 1.05 < g/f)
	    std::cout << "warning: "
		      << "frequency given:" << g << ' '
		      << "but measured:"    << f << '\n' ;
	f = g ;
    }
    else std::cout << "info: measured frequency: " << f << '\n' ;
    // [todo] print this only in debug mode

    auto t = Bang::asTicks(getTiming(argL),f) ;
    Bang bang(rpi,datPin,loadPin,clkPin,t) ;
  
    auto arg = argL->pop() ;
    if      (arg == "-d") dataInvoke(rpi,&bang,argL) ;
    else if (arg == "-s") fileInvoke(rpi,&bang,argL) ;

    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
