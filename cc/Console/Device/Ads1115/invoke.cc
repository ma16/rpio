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
    if (argL->empty())
    {
	auto record = host->readConfig() ;
	auto success = host->verify(record).success() ;
	std::cout << (success ? "success: " : "error: ") ;
	std::cout << std::hex << host->fetch(record) << '\n' ;
    }
    else
    {
	auto word = Ui::strto<uint16_t>(argL->pop()) ;
	argL->finalize() ;
	auto record = host->writeConfig(word) ;
	auto success = host->verify(record).success() ;
	std::cout << (success ? "success\n" : "error\n") ;
    }
}

static void resetInvoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    argL->finalize() ;
    auto record = host->reset() ;
    auto success = host->verify(record).success() ;
    std::cout << (success ? "success\n" : "error\n") ;
}
    
static void sampleInvoke(Device::Ads1115::Bang *host,Ui::ArgL *argL)
{
    argL->finalize() ;
    auto record = host->readSample() ;
    auto success = host->verify(record).success() ;
    std::cout << (success ? "success: " : "error: ") ;
    std::cout << std::hex << host->fetch(record) << '\n' ;
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
	ticks(::Device::Ads1115::Bang::fast_timing().  buf),
	ticks(::Device::Ads1115::Bang::fast_timing().hdsta),
	ticks(::Device::Ads1115::Bang::fast_timing().susto),
	ticks(::Device::Ads1115::Bang::fast_timing().sudat),
	ticks(::Device::Ads1115::Bang::fast_timing().hddat),
	ticks(::Device::Ads1115::Bang::fast_timing().  low),
	ticks(::Device::Ads1115::Bang::fast_timing(). high)) ;

    std::cerr << timing.buf << ' '
	      << timing.hdsta << ' '
	      << timing.susto << ' '
	      << timing.sudat << ' '
	      << timing.hddat << ' '
	      << timing.low << ' '
	      << timing.high << '\n' ;
    
    ::Device::Ads1115::Bang host(rpi,sclPin,sdaPin,addr,timing) ;
    // ...[future] make timings optional arguments

    /* [todo]
    this->gpio.setOutput<Lo>(this->sclPin) ;
    this->gpio.setOutput<Lo>(this->sdaPin) ;
  
    // make sure the ARM counter is on and runs at 100 Mhz
    if (!this->counter.enabled())
	throw std::runtime_error("please enable ARM counter") ;
    if (this->counter.prescaler() != 3)
	throw std::runtime_error("please set ARM prescaler to <3>") ;
    */
    
    auto arg = argL->pop() ;
    if (false) ;
    
    else if (arg == "config") configInvoke(&host,argL) ;
    else if (arg ==  "reset")  resetInvoke(&host,argL) ;
    else if (arg == "sample") sampleInvoke(&host,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
