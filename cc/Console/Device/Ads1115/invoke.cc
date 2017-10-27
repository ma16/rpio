// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ads1115/Bang/Host.h>
#include <Device/Ads1115/Bang/Generator.h>
#include "../invoke.h"
#include <Neat/cast.h>
#include <Ui/strto.h>
#include <chrono>
#include <deque>
#include <iostream>
#include <math.h>

using namespace Device::Ads1115 ; // Circuit
using namespace Bang ; // Config,Record,Generator,Host

static void rateInvoke(Rpi::Peripheral *rpi,Config const &config,Ui::ArgL *argL)
{
    auto n = Ui::strto<size_t>(argL->pop()) ;
    argL->finalize() ;

    Generator gen(config) ;
    
    Record::Write setup_record ;
    auto setup_script = gen.writeConfig(&setup_record,0x8583) ;
    
    Record::Read status_record ;
    auto status_script = gen.readConfig(&status_record) ;

    Record::Read sample_record ;
    auto sample_script = gen.readSample(&sample_record) ;

    RpiExt::Bang scheduler(rpi) ;
    
    int64_t total = 0 ;
    auto t0 = std::chrono::steady_clock::now() ;
    for (decltype(n) i=0 ; i<n ; ++i)
    {
	// [todo] check for errors
	scheduler.execute(setup_script) ;
	do scheduler.execute(status_script) ;
	while (0 == (0x8000 & status_record.fetch(config.sdaPin))) ;
	// [todo] incorporate timeout
	scheduler.execute(sample_script) ;
	total += static_cast<int16_t>(sample_record.fetch(config.sdaPin)) ;
    }
    auto t1 = std::chrono::steady_clock::now() ;

    auto rate = n/std::chrono::duration<double>(t1-t0).count() ;
    auto avg = 2.048 / 0x8000 * static_cast<double>(total) / n  ;

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << rate << "/s (" << avg << ")\n" ;
}
    
static void configInvoke(Rpi::Peripheral *rpi,Config const &config,Ui::ArgL *argL)
{
    Host host(rpi,config) ;
    if (argL->empty())
    {
	auto record = host.readConfig() ;
	auto success = record.verify(config.sdaPin).success() ;
	std::cout << (success ? "success: " : "error: ") ;
	std::cout << std::hex << record.fetch(config.sdaPin) << '\n' ;
    }
    else
    {
	auto word = Ui::strto<uint16_t>(argL->pop()) ;
	argL->finalize() ;
	auto record = host.writeConfig(word) ;
	auto success = record.verify(config.sdaPin).success() ;
	std::cout << (success ? "success\n" : "error\n") ;
    }
}

static void resetInvoke(Rpi::Peripheral *rpi,Config const &config,Ui::ArgL *argL)
{
    argL->finalize() ;
    Host host(rpi,config) ;
    auto record = host.reset() ;
    auto success = record.verify(config.sdaPin).success() ;
    std::cout << (success ? "success\n" : "error\n") ;
}
    
static void sampleInvoke(Rpi::Peripheral *rpi,Config const &config,Ui::ArgL *argL)
{
    argL->finalize() ;
    Host host(rpi,config) ;
    auto record = host.readSample() ;
    auto success = record.verify(config.sdaPin).success() ;
    std::cout << (success ? "success: " : "error: ") ;
    std::cout << static_cast<int16_t>(record.fetch(config.sdaPin)) << '\n' ;
}

void Console::Device::Ads1115::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") {
	std::cout << "arguments: SCL SDA ADDR [-m] MODE\n"
		  << '\n'
		  << " SCL: Pi's pin to set the ADS1115's SCL pin\n"
		  << " SDA: Pi's pin to r/w the ADS1115's SDA pin\n"
		  << "ADDR: 0..127\n"
		  << '\n'
		  << "MODE : config       # read configuration register\n"
		  << "     | config WORD  # write configuration register\n"
		  << "     | rate N       # determine sample-rate\n"
		  << "     | reset        # write reset command\n"
		  << "     | sample       # read sample register\n" ;
	return ;
    }
  
    auto sclPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    Rpi::Gpio::Output(rpi).clear().write(1u << sclPin.value()) ;
    
    auto sdaPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    Rpi::Gpio::Output(rpi).clear().write(1u << sdaPin.value()) ;
    
    auto addr = Ui::strto(argL->pop(),Circuit::Addr()) ;

    auto f = Rpi::ArmTimer(rpi).frequency() ;
    if (f < 1e+4)
	std::cerr << "warning:frequency too small: " << f << "/s\n" ;
    
    auto ticks = [f](float seconds) {
	return seconds <= 0.0 ? 0u :
	static_cast<uint32_t>(ceil(f * seconds + 1.5)) ; } ;
    // ...ceil since we have to guarantee a minimum delay
    // ...+0.5 to round
    // ...+1.0 since (clock[i+1]-clock[i]) can be zero
    Circuit::Timing<uint32_t> timing(
	ticks(Circuit::fast_timing().  buf),
	ticks(Circuit::fast_timing().hdsta),
	ticks(Circuit::fast_timing().susto),
	ticks(Circuit::fast_timing().sudat),
	ticks(Circuit::fast_timing().hddat),
	ticks(Circuit::fast_timing().  low),
	ticks(Circuit::fast_timing(). high)) ;
    // ...[future] make timings optional arguments
    
    Config config(sclPin,sdaPin,addr,timing) ;

    auto arg = argL->pop() ;
    if (false) ;
    
    else if (arg == "config") configInvoke(rpi,config,argL) ;
    else if (arg ==  "rate")    rateInvoke(rpi,config,argL) ;
    else if (arg ==  "reset")  resetInvoke(rpi,config,argL) ;
    else if (arg == "sample") sampleInvoke(rpi,config,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
