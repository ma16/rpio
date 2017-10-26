// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ws2812b/BitStream.h>
#include <Device/Ws2812b/Circuit.h>
#include <RpiExt/Pwm.h>
#include <RpiExt/Serialize.h>
#include <RpiExt/Spi0.h>
#include <Ui/strto.h>
#include <iostream>

// --------------------------------------------------------------------

static Device::Ws2812b::Circuit::Seconds getBang(Ui::ArgL *argL)
{
    if (argL->pop_if("-t"))
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
  
static std::vector<RpiExt::Serialize::Edge> make(
    size_t nleds,uint32_t grb,uint32_t pins,
    Device::Ws2812b::Circuit::Ticks const &ticks)
{
    auto const Lo = Rpi::Gpio::Output::Lo ;
    auto const Hi = Rpi::Gpio::Output::Hi ;

    using Edge = RpiExt::Serialize::Edge ;
    std::deque<Edge> q ;

    // [debugging] make start visible in analyser
    q.push_back(Edge(pins,10,~0u,Lo)) ;
    q.push_back(Edge(pins,10,~0u,Hi)) ;
    
    // reset
    q.push_back(Edge(pins,10,~0u,Lo)) ;

    auto min = ticks.res_min ;
    auto max = ~0u ;

    // for each LED
    for (size_t i=0 ; i<nleds ; ++i)
    {
	// for each LED-bit
	for (uint32_t mask=(1u<<23) ; mask!=0 ; mask>>=1)
	{
	    q.push_back(Edge(pins,min,max,Hi)) ;
	    auto bit = grb & mask ;
	    if (bit)
	    {
		q.push_back(Edge(pins,ticks.t1h_min,ticks.t1h_max,Lo)) ;
		min = ticks.t1l_min ;
		max = ticks.t1l_max ;
	    }
	    else
	    {
		q.push_back(Edge(pins,ticks.t0h_min,ticks.t0h_max,Lo)) ;
		min = ticks.t0l_min ;
		max = ticks.t0l_max ;
	    }
	}
    }
    q.push_back(Edge(pins,ticks.res_min,~0u,Lo)) ;

    return std::vector<Edge>(q.begin(),q.end()) ;
}

static void bang(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    Rpi::Gpio gpio(rpi) ;
    Rpi::ArmTimer timer(rpi) ;
    
    auto nleds = Ui::strto<unsigned>(argL->pop()) ;
    auto   grb = Ui::strto<unsigned>(argL->pop()) ;
  
    auto timing = getBang(argL) ;
    auto freq = timer.frequency() ;
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto max_retries = Ui::strto<uint64_t>(argL->option("-r","1")) ;
    auto debug = argL->pop_if("-d") ;
    argL->finalize() ;
  
    auto ticks = Device::Ws2812b::Circuit::asTicks(timing,freq) ;
    if (debug)
    {
	std::cout.setf(std::ios::scientific) ;
	std::cout.precision(2) ;
	std::cout << "f=" << freq << '\n'
		  << "timing (seconds)="
		  << Device::Ws2812b::Circuit::toStr(timing) << '\n'
		  << "timing (ticks)="
		  << Device::Ws2812b::Circuit::toStr(ticks)  << '\n' ;
    }
  
    RpiExt::Serialize host(gpio,timer) ;
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

static Device::Ws2812b::BitStream::Seconds getPwm(Ui::ArgL *argL)
{
    using Seconds = Device::Ws2812b::BitStream::Seconds ;
    if (argL->pop_if("-t"))
    {
	Seconds s(
	    Seconds::Pulse(
		Ui::strto<float>(argL->pop()),
		Ui::strto<float>(argL->pop())),
	    Seconds::Pulse(
		Ui::strto<float>(argL->pop()),
		Ui::strto<float>(argL->pop())),
	    Ui::strto<float>(argL->pop())) ;
	return s ;
    }

    return Device::Ws2812b::BitStream::strict() ;
}
  
static Device::Ws2812b::BitStream::Ticks computeTicks(
    Device::Ws2812b::BitStream::Seconds const &s,double f)
{
    auto round = [f](float s) {
	return static_cast<size_t>(floor(f * s + 0.5)) ; } ;
    using Ticks = Device::Ws2812b::BitStream::Ticks ;
    return Ticks(
	Ticks::Pulse(round(s.bit_0.hi),round(s.bit_0.lo)),
	Ticks::Pulse(round(s.bit_1.hi),round(s.bit_1.lo)),
	round(s.reset)) ;
}

static Device::Ws2812b::BitStream::Seconds computeSeconds(
    Device::Ws2812b::BitStream::Ticks const &t,double f)
{
    auto round = [f](size_t t) {
	return static_cast<float>(t/f) ; } ;
    using Seconds = Device::Ws2812b::BitStream::Seconds ;
    return Seconds(
	Seconds::Pulse(round(t.bit_0.hi),round(t.bit_0.lo)),
	Seconds::Pulse(round(t.bit_1.hi),round(t.bit_1.lo)),
	round(t.reset)) ;
}

static void pwm(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto nleds = Ui::strto<unsigned>(argL->pop()) ;
    auto   grb = Ui::strto<unsigned>(argL->pop()) ;
    auto seconds = getPwm(argL) ;
    auto f = Ui::strto<double>(argL->pop()) ;
    auto debug = argL->pop_if("-d") ;
    argL->finalize() ;

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    
    auto ticks = computeTicks(seconds,f) ;
    if (debug)
    {
	Device::Ws2812b::BitStream::Seconds effective = computeSeconds(ticks,f) ;
	std::cout << "f=" << f << '\n'
		  << "timing (seconds)="    <<   seconds.toStr() << '\n'
		  << "timing (ticks)="      <<     ticks.toStr() << '\n' 
		  << "effective (seconds)=" << effective.toStr() << '\n' ;
    }
    auto v = Device::Ws2812b::BitStream::make32(ticks,grb,nleds) ;
    auto n =  RpiExt::Pwm(rpi).convey(&v[0],v.size(),0) ;
    if (v.size() != n)
	std::cout << "failure (" << n << "/" << v.size() << ")\n" ;
}

// --------------------------------------------------------------------

static void spi0(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto nleds = Ui::strto<unsigned>(argL->pop()) ;
    auto   grb = Ui::strto<unsigned>(argL->pop()) ;
  
    auto seconds = getPwm(argL) ;
    auto f = Ui::strto<double>(argL->pop()) ;
    // [todo] measure/guess frequency
    // [todo] debug
    argL->finalize() ;
    auto ticks = computeTicks(seconds,f) ;
    auto v = Device::Ws2812b::BitStream::make8(ticks,grb,nleds) ;
    RpiExt::Spi0(rpi).xfer(v) ;
    // [todo] we may encounter a timeout (process suspension)
}

// --------------------------------------------------------------------

void Console::Device::Ws2812b::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: MODE\n"
		  << '\n'
		  << "MODE : bang NLEDS GRB [-t TIMING] [-f FREQ] PINS [-r RETRY] [-d]\n"
		  << "     | pwm  NLEDS GRB [-t TIMING] FREQ [-d]\n"
		  << "     | spi0 NLEDS GRB [-t TIMING] FREQ\n"
		  << '\n'
		  << "TIMING : -t T1..T5\n"
		  << '\n'
		  << "T1 = the 0-bit's High-level duration\n"
		  << "T2 = the 0-bit's  Low-level duration\n"
		  << "T3 = the 1-bit's High-level duration\n"
		  << "T4 = the 1-bit's  Low-level duration\n"
		  << "T5 = the latch (reset) duration\n"
		  << "Min and max values have to be provided for T1..T4 in bit-banged mode\n"
		  << '\n'
		  << "All values in seconds\n"
		  << "Default values are the ones on the datasheet.\n"
		  << '\n'
		  << "-d : display debug information\n"
		  << std::flush ;
	// [todo] create files, read from file, read chains, read various chains
	return ;
    }

    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "bang") bang(rpi,argL) ;
    else if (arg ==  "pwm")  pwm(rpi,argL) ;
    else if (arg == "spi0") spi0(rpi,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
