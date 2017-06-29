// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"

#include <chrono>
#include <deque>
#include <iostream>

#include <Device/Mcp3008/Bang.h>
#include <Device/Mcp3008/Spi0.h>
#include <Device/Mcp3008/Spi1.h>
#include <Neat/cast.h>
#include <Posix/base.h>
#include <Ui/strto.h>

using namespace Device::Mcp3008 ;

// --------------------------------------------------------------------

static void bangRate(Rpi::Peripheral *rpi,Bang *host,bool monitor,Ui::ArgL *argL)
{
    auto n = Ui::strto<size_t>(argL->pop()) ;
    auto source = Ui::strto(argL->option("-s","0"),Circuit::Source()) ;
    argL->finalize() ;

    Bang::Record record ;
    auto script = host->makeScript(source,monitor,&record) ;
    
    RpiExt::Bang scheduler(rpi) ;
    std::vector<Bang::Record> v(n) ;
    
    auto t0 = std::chrono::steady_clock::now() ;
    for (decltype(n) i=0 ; i<n ; ++i)
    {
	scheduler.execute(script.begin(),script.end()) ;
	v[i] = record ;
	// the alternative is to compile a long script which already
	// includes all the iterations. this is however not that fast
	// (maybe the cache prediction isn't good enough for this).
    }
    auto t1 = std::chrono::steady_clock::now() ;
    
    if (monitor)
    {
	decltype(n) array[0x100] = { 0 } ;
	for (decltype(n) i=0 ; i<n ; ++i)
	{
	    uint8_t code = host->error(v[i]).value() ;
	    ++array[code] ;
	}
	for (unsigned code=1 ; code<0x100 ; ++code)
	{
	    if (array[code] > 0)
		std::cout << "error 0x" << std::hex << code << ": "
			  << std::dec << array[code] << '\n' ;
	}
	std::cout << "success: " << array[0] << '\n' ;
    }
    auto rate = n/std::chrono::duration<double>(t1-t0).count() ;
    std::cout << rate << "/s" << std::endl ;
}
    
static void bangSample(Rpi::Peripheral*,Bang *host,bool monitor,Ui::ArgL *argL)
{
    auto source = Ui::strto(argL->pop(),Circuit::Source()) ;
    std::deque<decltype(source)> q(1,source) ;
    while (!argL->empty())
    {
	q.push_back(Ui::strto(argL->pop(),Circuit::Source())) ;
    }
    argL->finalize() ;
    for (auto source: q)
    {
	auto sample = host->query(source,monitor) ;
	if (monitor)
	{
	    std::cout << '('
		      << std::dec << host->msb(sample).value() << ',' 
		      << host->lsb(sample).value() << ',' 
		      << std::hex << (unsigned)host->error(sample).value()
		      << ") " ;
	}
	else std::cout << host->msb(sample).value() << ' ' ;
    }
    std::cout << std::endl ;
}

void bangInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: CS DIN DOUT CLK [-m] [-f FREQ] MODE\n"
		  << '\n'
		  << "  CS: Pi's pin to feed MCP's   CS pin @10\n"
		  << " DIN: Pi's pin to feed MCP's  DIN pin @11\n"
		  << "DOUT: Pi's pin to read MCP's DOUT pin @12\n"
		  << " CLK: Pi's pin to feed MCP's  CLK pin @13\n"
		  << '\n'
		  << "-m: enable monitoring to detect communication problems\n"
		  << '\n'
		  << "FREQ: ARM counter frequency that has been set up\n"
		  << '\n'
		  << "MODE : rate N [-s SOURCE]  # perform throughput test\n"
		  << "     | sample SOURCE+      # read one or more samples\n" 
		  << '\n'
		  << "N - the number of consecutive samples to take\n" 
		  << "SOURCE - the MCP3008-channel to sample (0..15)\n" ;
	return ;
    }
  
    auto   csPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto  dinPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto doutPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto  clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

    auto monitor = argL->pop_if("-m") ;

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

    Bang::Timing timing ;
    // ...[future] make timings optional arguments
    auto ticks = [f](double d) {
	return static_cast<uint32_t>(ceil(f * d + 1.5)) ; } ;
    // ...ceil since we have to guarantee a minimum delay
    // ...+0.5 to round
    // ...+1.0 since (clock[i+1]-clock[i]) can be zero
    timing.csh  = ticks(Circuit::timing_3v.csh ) ;
    timing.sucs = ticks(Circuit::timing_3v.sucs) ;
    timing.hd   = ticks(Circuit::timing_3v.hd  ) ;
    timing.su   = ticks(Circuit::timing_3v.su  ) ;
    timing.en   = ticks(Circuit::timing_3v.en  ) ;
    timing.lo   = ticks(Circuit::timing_3v.lo  ) ;
    timing.hi   = ticks(Circuit::timing_3v.hi  ) ;
    timing.dov  = ticks(Circuit::timing_3v.dov ) ;
    timing.dis  = ticks(Circuit::timing_3v.dis ) ;
    timing.bled = ticks(Circuit::timing_3v.bled) ;

    Bang host(rpi,csPin,clkPin,dinPin,doutPin,timing) ;

    auto arg = argL->pop() ;
    if (false) ;
    
    else if (arg ==   "rate")   bangRate(rpi,&host,monitor,argL) ;
    else if (arg == "sample") bangSample(rpi,&host,monitor,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

// --------------------------------------------------------------------

static void spi0Rate(Spi0 *host,bool monitor,Ui::ArgL *argL)
{
    auto n = Ui::strto<unsigned>(argL->pop()) ;
    auto source = Ui::strto(argL->option("-s","0"),Circuit::Source()) ;
    argL->finalize() ;

    std::chrono::steady_clock::time_point t0,t1 ;
    if (monitor)
    {
	std::vector<Device::Mcp3008::Spi0::Sample32> v(n) ;
	t0 = std::chrono::steady_clock::now() ;
	for (decltype(n) i=0 ; i<n ; ++i)
	    v[i] = host->query32(source) ;
	t1 = std::chrono::steady_clock::now() ;
	decltype(n) array[0x8] = { 0 } ;
	for (decltype(n) i=0 ; i<n ; ++i)
	{
	    Neat::uint<unsigned,3> code = v[i].verify().code() ;
	    ++array[code.value()] ;
	}
	for (unsigned code=1 ; code<0x8 ; ++code)
	{
	    if (array[code] > 0)
		std::cout << "error 0x" << std::hex << code << ": "
			  << std::dec << array[code] << '\n' ;
	}
	std::cout << "success: " << array[0] << '\n' ;
    }
    else
    {
	t0 = std::chrono::steady_clock::now() ;
	for (decltype(n) i=0 ; i<n ; ++i)
	    host->query24(source) ;
	t1 = std::chrono::steady_clock::now() ;
    }
    
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << n/std::chrono::duration<double>(t1-t0).count() << "/s" << std::endl ;
}
    
static void spi0Sample(Spi0 *host,bool monitor,Ui::ArgL *argL)
{
    auto source = Ui::strto(argL->pop(),Circuit::Source()) ;
    std::deque<decltype(source)> q(1,source) ;
    while (!argL->empty())
    {
	q.push_back(Ui::strto(argL->pop(),Circuit::Source())) ;
    }
    argL->finalize() ;
    for (auto source: q)
    {
	if (monitor)
	{
	    auto sample = host->query32(source) ;
	    if (!sample.verify().ok())
		std::cout << "Error code <" << 0 << '>' << std::endl ;
	    std::cout << sample.fetch().value() << " " ;
	}
	else std::cout << host->query24(source).fetch().value() << " " ;
    }
    std::cout << std::endl ;
}
    
static void spi0Invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: [-m] MODE [help]\n"
		  << '\n'
		  << "-m: enable monitoring to detect communication problems\n"
		  << '\n'
		  << "MODE : rate N [-s SOURCE]  # perform throughput test\n"
		  << "     | sample SOURCE+      # read one or more samples\n" 
		  << '\n'
		  << "N - the number of consecutive samples to take\n" 
		  << "SOURCE - the MCP3008-channel to sample (0..15)\n" ;
	return ;
    }
  
    auto monitor = argL->pop_if("-m") ;
    Spi0 host(rpi) ;

    auto arg = argL->pop() ;
    if (false) ;
    
    else if (arg ==   "rate")   spi0Rate(&host,monitor,argL) ;
    else if (arg == "sample") spi0Sample(&host,monitor,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

// --------------------------------------------------------------------

static void spi1Sample(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    // note: pin mode (ALT4) has to be enabled by user
    // i.e. CS0 (18), CS1 (17), CS2 (16), SCLK (21), MISO (19), MOSI (20)
    
    if (!argL->empty() && (argL->peek() == "help"))
    {
	std::cout << "arguments: [-m] [-s SPEED] [-c CS] MODE\n"
		  << "\n"
		  << "   -m: enable monitoring to detect communication errors\n"
		  << "SPEED: divider to define SCLK speed (default 49)\n"
		  << "   CS: bit-mask (0..7) for chip-select pins (default 4 for CS#0)\n"
		  << "\n"
		  << "MODE : rate N [-s SOURCE]  # perform throughput test\n"
		  << "     | sample SOURCE+      # read one or more samples\n" 
		  << '\n'
		  << "SOURCE - the MCP3008-channel to sample (0..15)\n" ;
	return ;
    }
    auto monitor = argL->pop_if("-m") ;
    auto speed = Ui::strto(argL->option("-s","49"),Spi1::Speed()) ;
    // ...on Pi-0: 4 Mhz SCLK, 200 khz sample rate
    auto cs = Ui::strto(argL->option("-c","4"),Spi1::Cs()) ;
    // ...CS#0 (pin 18)
    argL->finalize() ;

    // prepare room for tx/rx data
    std::deque<uint32_t> tx ;
    for (Circuit::Source::Unsigned i=0 ; i<16 ; ++i)
    {
	auto source = Circuit::Source::make(i) ;
	tx.push_back(Spi1::makeTx(source)) ;
    }
    std::vector<uint32_t> rx ; rx.reserve(tx.size()) ;

    // init SPI
    Rpi::Spi1 spi(rpi) ;
    Spi1::setup(&spi,speed,cs,monitor) ;

    // transceive data
    spi.xfer(tx,&rx,false) ; // [todo] arrays are more generic, see Spi0

    // depending on the monitoring flag, we get 17- or 26-bit words
    for (auto w: rx)
    {
	if (monitor)
	{
	    Circuit::Sample sample ;
	    auto result = Spi1::evalRx(w,&sample) ;
	    if (result == Spi1::Success)
	    {
		std::cout << sample.value() << ' ' ;
	    }
	    else std::cout << "error(" << result << ") " ;
	}
	else std::cout << Spi1::evalRx(w).value() << ' ' ;
    }
    std::cout << std::endl ;
}
    
// --------------------------------------------------------------------

void Console::Device::Mcp3008::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: ( bang | spi0 | spi1 ) [help]\n"
		  << "\n"
		  << "bang: use bit-banging\n"
		  << "spi0: use main SPI controller\n"
		  << "spi1: use auxilliary SPI controller\n"
		  << "\n"
		  << "SOURCE :  0  # differential CH0 = IN+ CH1 = IN-\n"
		  << "       |  1  # differential CH0 = IN- CH1 = IN+\n"
		  << "       |  2  # differential CH2 = IN+ CH3 = IN-\n"
		  << "       |  3  # differential CH2 = IN- CH3 = IN+\n"
		  << "       |  4  # differential CH4 = IN+ CH5 = IN-\n"
		  << "       |  5  # differential CH4 = IN- CH5 = IN+\n"
		  << "       |  6  # differential CH6 = IN+ CH7 = IN-\n"
		  << "       |  7  # differential CH6 = IN- CH7 = IN+\n"
		  << "       |  8  # single-ended CH0\n"
		  << "       |  9  # single-ended CH1\n"
		  << "       | 10  # single-ended CH2\n"
		  << "       | 11  # single-ended CH3\n"
		  << "       | 12  # single-ended CH4\n"
		  << "       | 13  # single-ended CH5\n"
		  << "       | 14  # single-ended CH6\n"
		  << "       | 15  # single-ended CH7\n" ;
	    
	return ;
    }
    auto arg = argL->pop() ;
    if (false) ;
      
    else if (arg == "bang") bangInvoke(rpi,argL) ;
    else if (arg == "spi0") spi0Invoke(rpi,argL) ;
    else if (arg == "spi1") spi1Sample(rpi,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
