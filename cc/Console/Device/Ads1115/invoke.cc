// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ads1115/Bang.h>
#include <Neat/cast.h>
#include <Ui/strto.h>
#include <chrono>
#include <deque>
#include <iostream>

static void configInvoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    if (argL->empty()) {
	auto word = host->readConfig() ;
	if (word) {
	    std::cout << std::hex << (*word) << std::endl ;
	}
	else {
	    std::cout << "error" << std::endl ;
	}
    }
    else {
	auto word = Ui::strto<uint16_t>(argL->pop()) ;
	argL->finalize() ;
	auto success = host->writeConfig(word) ;
	if (!success) 
	    std::cout << "error" << std::endl ;
    }
}
    
static void config2Invoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    if (argL->empty())
    {
	auto record = host->readConfig2() ;
	unsigned word = 0 ;
	for (int i=0 ; i<2 ; ++i)
	    for (int j=0 ; j<8 ; ++j)
	    {
		word <<= 1 ;
		word |= 0 != (record.recv[i][j] & (1u << 23)) ;
	    }
	std::cout << std::hex << word << std::endl ;
	// todo
    }
    else
    {
	auto word = Ui::strto<uint16_t>(argL->pop()) ;
	argL->finalize() ;
	auto success = host->writeConfig2(word) ;
	if (!success) 
	    std::cout << "error" << std::endl ;
    }
}

static void resetInvoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    argL->finalize() ;
    auto success = host->doReset() ;
    std::cout << success << std::endl ;
}
    
static void sampleInvoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    argL->finalize() ;
    auto sample = host->readSample() ;
    if (sample) {
	std::cout << std::hex << (*sample) << std::endl ;
    }
    else {
	std::cout << "error" << std::endl ;
    }
}

void Console::Device::Ads1115::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: SCL SDA ADDR [-m] MODE\n"
		  << '\n'
		  << " SCL: Pi's pin to feed ADS's SCL pin\n"
		  << " SDA: Pi's pin to  r/w ADS's SDA pin\n"
		  << "ADDR: 0..127\n"
		  << '\n'
		  << "-m: enable monitoring to detect communication errors\n"
		  << '\n'
		  << "MODE : config       # read configuration register\n"
		  << "     | config WORD  # write configuration register\n"
		  << "     | reset        # write reset command\n"
		  << "     | sample       # read sample register\n"
		  << std::flush ;
	return ;
    }
  
    auto sclPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto sdaPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto addr = Ui::strto(argL->pop(),::Device::Ads1115::Bang::Addr()) ;
    auto monitor = argL->pop_if("-m") ;

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

    // ...[future] make timings optional arguments
    auto ticks = [f](float seconds) {
	return static_cast<uint32_t>(ceil(f * seconds + 1.5)) ; } ;
    // ...ceil since we have to guarantee a minimum delay
    // ...+0.5 to round
    // ...+1.0 since (clock[i+1]-clock[i]) can be zero
    ::Device::Ads1115::Bang::Timing<uint32_t> timing(
	ticks(::Device::Ads1115::Bang::default_timing().  buf),
	ticks(::Device::Ads1115::Bang::default_timing().hdsta),
	ticks(::Device::Ads1115::Bang::default_timing().susto),
	ticks(::Device::Ads1115::Bang::default_timing().sudat),
	ticks(::Device::Ads1115::Bang::default_timing().hddat),
	ticks(::Device::Ads1115::Bang::default_timing().  low),
	ticks(::Device::Ads1115::Bang::default_timing(). high),
	ticks(::Device::Ads1115::Bang::default_timing(). fall),
	ticks(::Device::Ads1115::Bang::default_timing(). rise)) ;
    // [todo] last two are maximum timings!
    
    ::Device::Ads1115::Bang host(rpi,sclPin,sdaPin,addr,timing,monitor) ;
    // ...[future] make timings optional arguments

    auto arg = argL->pop() ;
    if      (arg == "config") configInvoke(&host,argL) ;
    else if (arg ==  "reset")  resetInvoke(&host,argL) ;
    else if (arg == "sample") sampleInvoke(&host,argL) ;
    
    else if (arg == "config2") config2Invoke(&host,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
