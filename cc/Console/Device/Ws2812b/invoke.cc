// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include "Pwm.h"
#include "Spi0.h"
#include <RpiExt/Serialize.h>
#include <Ui/strto.h>
#include <iostream>

static std::vector<RpiExt::Serialize::Edge> make(
    size_t nleds,uint32_t grb,uint32_t pins,
    Console::Ws2812b::Timing::Ticks const &ticks)
{
    auto const Lo = Rpi::Gpio::Output::Lo ;
    auto const Hi = Rpi::Gpio::Output::Hi ;

    using Edge = RpiExt::Serialize::Edge ;
    std::deque<Edge> q ;
    
    // reset
    q.push_back(Edge(Lo,pins,0,~0u)) ;
    q.push_back(Edge(Hi,pins,ticks.res_min,~0u)) ;

    // for each LED
    for (size_t i=0 ; i<nleds ; ++i)
    {
	// for each LED-bit
	for (uint32_t mask=(1u<<23) ; mask!=0 ; mask>>=1)
	{
	    auto bit = grb & mask ;
	    if (bit)
	    {
		q.push_back(Edge(Lo,pins,ticks.t1h_min,ticks.t1h_max)) ;
		q.push_back(Edge(Hi,pins,ticks.t1l_min,ticks.t1l_max)) ;
	    }
	    else
	    {
		q.push_back(Edge(Lo,pins,ticks.t0h_min,ticks.t0h_max)) ;
		q.push_back(Edge(Hi,pins,ticks.t0l_min,ticks.t0l_max)) ;
	    }
	}
    }

    return std::vector<Edge>(q.begin(),q.end()) ;
}

static void bang(Rpi::Peripheral *rpi,
		 unsigned nleds,
		 unsigned grb,
		 Console::Ws2812b::Timing::Seconds const &timing,
		 Ui::ArgL *argL)
{
    Rpi::Gpio gpio(rpi) ;
    Rpi::Counter counter(rpi) ;
    // bit banging specific
    auto freq = (argL->pop_if("-f"))
	? Ui::strto<double>(argL->pop()) 
	: RpiExt::Serialize::frequency(counter) ;
    // [future] abort if frequency<1E+6
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto max = Ui::strto<uint64_t>(argL->option("-r","1")) ;
    // ...stop if nretries+1==max; infinite if zero
    auto debug = argL->pop_if("-d") ;
    argL->finalize() ;
  
    auto ticks = Console::Ws2812b::Timing::compute(timing,freq) ;
    RpiExt::Serialize host(gpio,counter) ;
    std::vector<RpiExt::Serialize::Edge> v =
	make(nleds,grb,(1u<<pin.value()),ticks) ;
  
    if (debug) {
	std::cout << "f=" << freq << '\n'
		  << "timing (seconds)=" << Console::Ws2812b::Timing::toStr(timing) << '\n'
		  << "timing (ticks)=" << Console::Ws2812b::Timing::toStr(ticks) << '\n' ;
    }
  
    decltype(max) retries = 0 ;
    bool success ;
    do success = host.send(v) ;
    while (retries++ < max && !success) ;
    if (debug)
	std::cout << "retries: " << (retries-1) << "\n" ;
}

static void pwm(Rpi::Peripheral *rpi,
		unsigned nleds,
		unsigned grb,
		Console::Ws2812b::Timing::Seconds const &timing,
		Ui::ArgL *argL)
{
  auto channel = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ;
  auto freq = Ui::strto<double>(argL->pop()) ;
  auto debug = argL->pop_if("-d") ;
  argL->finalize() ;
  auto nticks = Console::Ws2812b::Timing::computeNticks(timing,freq) ;
  if (debug) {
    std::cout << "f=" << freq << '\n'
	      << "timing (seconds)=" << Console::Ws2812b::Timing::toStr(timing) << '\n'
	      << "timing (ticks)=" << Console::Ws2812b::Timing::toStr(nticks) << '\n' ;
  }
  Console::Ws2812b::Pwm pwm(rpi,channel,nticks) ;
  pwm.send(nleds,grb) ;
}

static void spi0(Rpi::Peripheral *rpi,
		 unsigned nleds,
		 unsigned grb,
		 Console::Ws2812b::Timing::Seconds const&,
		 Ui::ArgL *argL)
{
  auto tps = Ui::strto<double>(argL->pop()) ;
  argL->finalize() ;
  Console::Ws2812b::Spi0 spi(Rpi::Spi0(rpi),Console::Ws2812b::Spi0::Timing(),tps) ;
  spi.send(nleds,grb) ;
}

void Console::Device::Ws2812b::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: NLEDS GRB TIMING MODE\n"
	      << '\n'
	      << "MODE : bang PINS [-f FREQ] [-r RETRY] [-d]\n"
	      << "     | pwm FREQ CHANNEL [-d]\n"
	      << "     | spi0 TPS\n"
	      << '\n'
	      << "TIMING : --relaxed | (--timing T1..T9)\n"
	      << '\n'
	      << "T1 = the 0-bit's High-level minimum-duration\n"
	      << "T2 = the 0-bit's High-level maximum-duration\n"
	      << "T3 = the 0-bit's  Low-level minimum-duration\n"
	      << "T4 = the 0-bit's  Low-level maximum-duration\n"
	      << "T5 = the 1-bit's High-level minimum-duration\n"
	      << "T6 = the 1-bit's High-level maximum-duration\n"
	      << "T7 = the 1-bit's  Low-level minimum-duration\n"
	      << "T8 = the 1-bit's  Low-level maximum-duration\n"
	      << "T9 = the latch (reset) duration\n"
	      << '\n'
	      << "All values in seconds\n"
	      << "--relaxed = (experimental) relaxed timing values\n"
	      << "Otherwise the values from the datasheet are taken\n"
	      << std::flush ;
    // [todo] read from file, read chains, read various chains
    return ;
  }

  auto nleds = Ui::strto<unsigned>(argL->pop()) ;
  auto   grb = Ui::strto<unsigned>(argL->pop()) ;
  
  auto timing = Console::Ws2812b::Timing::get(argL) ;
  
  std::string arg = argL->pop() ;
  if (false) ;
  
  else if (arg == "bang") bang(rpi,nleds,grb,timing,argL) ;
  else if (arg ==  "pwm")  pwm(rpi,nleds,grb,timing,argL) ;
  else if (arg == "spi0") spi0(rpi,nleds,grb,timing,argL) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
