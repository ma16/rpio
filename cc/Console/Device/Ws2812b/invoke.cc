// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ws2812b/BitStream.h>
#include <Device/Ws2812b/Circuit.h>
#include <Device/Ws2812b/Spi0.h>
#include <RpiExt/Pwm.h>
#include <RpiExt/Serialize.h>
#include <Ui/strto.h>
#include <iostream>

// --------------------------------------------------------------------

static std::vector<RpiExt::Serialize::Edge> make(
    size_t nleds,uint32_t grb,uint32_t pins,
    Device::Ws2812b::Circuit::Ticks const &ticks)
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
		 Device::Ws2812b::Circuit::Seconds const &timing,
		 Ui::ArgL *argL)
{
    Rpi::Gpio gpio(rpi) ;
    Rpi::Counter counter(rpi) ;
    
    auto freq = counter.frequency() ;
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto max_retries = Ui::strto<uint64_t>(argL->option("-r","1")) ;
    auto debug = argL->pop_if("-d") ;
    argL->finalize() ;
  
    auto ticks = Device::Ws2812b::Circuit::asTicks(timing,freq) ;
    if (debug)
    {
	std::cout << "f=" << freq << '\n'
		  << "timing (seconds)="
		  << Device::Ws2812b::Circuit::toStr(timing) << '\n'
		  << "timing (ticks)="
		  << Device::Ws2812b::Circuit::toStr(ticks)  << '\n' ;
    }
  
    RpiExt::Serialize host(gpio,counter) ;
    std::vector<RpiExt::Serialize::Edge> v =
	make(nleds,grb,(1u<<pin.value()),ticks) ;
  
    decltype(max_retries) i = 0 ;
    bool success ;
    do {
	++i ;
	success = host.send(v) ;
    }
    while (!success && ((max_retries==0) || (i<max_retries))) ;
    if (!success)
	std::cout << "failed\n" ;
    if (debug)
	std::cout << "iterations: " <<  i << "\n" ;
}

// --------------------------------------------------------------------

static std::string toStr(Device::Ws2812b::BitStream::Ticks const &n)
{
    std::ostringstream os ;
    os << "0-bit:(" << n.t0h << ',' << n.t0l << ") 1-bit:(" << n.t1h << ',' << n.t1l << ") latch:" << n.res ;
    return os.str() ;
}
  
static Device::Ws2812b::BitStream::Ticks computeTicks(
    Device::Ws2812b::Circuit::Seconds const &s,double f)
{
    Device::Ws2812b::BitStream::Ticks n = {
	static_cast<uint32_t>((s.t0h_min+s.t0h_max)/2*f+0.5),
	static_cast<uint32_t>((s.t0l_min+s.t0l_max)/2*f+0.5),
	static_cast<uint32_t>((s.t1h_min+s.t1h_max)/2*f+0.5),
	static_cast<uint32_t>((s.t1l_min+s.t1l_max)/2*f+0.5),
	static_cast<uint32_t>(ceil(s.res_min*f)),
    } ;
#if 1   
    // ...[todo] make sure no overflows occur
    if (((n.t0h/f < s.t0h_min) || (s.t0h_max < n.t0h/f)) ||
	((n.t0l/f < s.t0l_min) || (s.t0l_max < n.t0l/f)) ||
	((n.t1h/f < s.t1h_min) || (s.t1h_max < n.t1h/f)) ||
	((n.t1l/f < s.t1l_min) || (s.t1l_max < n.t1l/f)))
	throw std::runtime_error("Ticks:cannot compute") ;
#endif
    return n ;
}

static void pwm(Rpi::Peripheral *rpi,
		unsigned nleds,
		unsigned grb,
		Device::Ws2812b::Circuit::Seconds const &timing,
		Ui::ArgL *argL)
{
    auto channel = Ui::strto(argL->pop(),Rpi::Pwm::Index()) ;
    auto freq = Ui::strto<double>(argL->pop()) ;
    auto debug = argL->pop_if("-d") ;
    argL->finalize() ;
    auto ticks = computeTicks(timing,freq) ;
    if (debug)
    {
	std::cout << "f=" << freq << '\n'
		  << "timing (seconds)="
		  << Device::Ws2812b::Circuit::toStr(timing) << '\n'
		  << "timing (ticks)=" << toStr(ticks) << '\n' ;
    }
    // [todo] make sure there is a clock-pulse
    auto v = Device::Ws2812b::BitStream::make(ticks,grb,nleds) ;
    RpiExt::Pwm(rpi,channel).send(v) ;
}

// --------------------------------------------------------------------

static void spi0(Rpi::Peripheral *rpi,
		 unsigned nleds,
		 unsigned grb,
		 Device::Ws2812b::Circuit::Seconds const&,
		 Ui::ArgL *argL)
{
    auto tps = Ui::strto<double>(argL->pop()) ;
    argL->finalize() ;
    Device::Ws2812b::Spi0 spi(Rpi::Spi0(rpi),Device::Ws2812b::Spi0::Timing(),tps) ;
    spi.send(nleds,grb) ;
}

// --------------------------------------------------------------------

static Device::Ws2812b::Circuit::Seconds get(Ui::ArgL *argL)
{
    if (argL->pop_if("--timing"))
    {
	Device::Ws2812b::Circuit::Seconds s(
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop()),
	    Ui::strto<float>(argL->pop())) ;
	return s ;
    }

    return Device::Ws2812b::Circuit::strict ;
}
  
void Console::Device::Ws2812b::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: NLEDS GRB [TIMING] MODE\n"
		  << '\n'
		  << "MODE : bang PINS [-r RETRY] [-d]\n"
		  << "     | pwm CHANNEL FREQ [-d]\n"
		  << "     | spi0 TPS\n"
		  << '\n'
		  << "TIMING : --timing T1..T9\n"
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
		  << "Default values are the ones on the datasheet.\n"
		  << '\n'
		  << "-d : display some debug information\n"
		  << std::flush ;
	// [todo] read from file, read chains, read various chains
	return ;
    }

    auto nleds = Ui::strto<unsigned>(argL->pop()) ;
    auto   grb = Ui::strto<unsigned>(argL->pop()) ;
  
    auto timing = get(argL) ;
  
    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "bang") bang(rpi,nleds,grb,timing,argL) ;
    else if (arg ==  "pwm")  pwm(rpi,nleds,grb,timing,argL) ;
    else if (arg == "spi0") spi0(rpi,nleds,grb,timing,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
