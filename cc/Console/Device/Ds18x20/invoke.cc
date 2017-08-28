// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Ui/strto.h>

static unsigned long long to_ull(std::vector<bool> const &v)
{
    unsigned long long ull = 0 ;
    for (unsigned i=v.size() ; i>0 ; )
    {
	ull <<= 1 ;
	--i ;
	ull |= v[i] ;
    }
    return ull ;
}

static std::ostream& operator<< (std::ostream &os,std::vector<bool> const &v)
{
    for (auto i=0u ; i<v.size() ; ++i)
    {
	os << v[i] ;
	if ((i % 8) == 7) os << ' ' ;
	else if ((i % 4) == 3) os << ':' ;
    }
    return os ;
}

static void convert(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    RpiExt::Bang::Stack stack(0x100) ;
    Bang::Record record ;
    RpiExt::Bang scheduler(rpi) ;

    auto script = Bang(rpi,pin).convert(&record,&stack) ;
    auto success = scheduler.execute(script) ;
    if (!success)
	std::cerr << "timing failed\n" ;

    // debugging
    auto v = Bang::assemble(record.buffer,72,1u<<pin.value()) ; 
    std::cout << v << '\n' ;
    v = Bang::assemble(record.buffer,64,1u<<pin.value()) ; 
    std::cout << std::hex << (unsigned)Bang::crc(v) << '\n' ;
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    RpiExt::Bang::Stack stack(0x100) ;
    Bang::Record record ;
    RpiExt::Bang scheduler(rpi) ;

    auto script = Bang(rpi,pin).readPad(&record,&stack) ;
    auto success = scheduler.execute(script) ;
    if (!success)
	std::cerr << "timing failed\n" ;

    // debugging
    auto v = Bang::assemble(record.buffer,72,1u<<pin.value()) ; 
    std::cout << v << '\n' ;
    v = Bang::assemble(record.buffer,64,1u<<pin.value()) ; 
    std::cout << std::hex << (unsigned)Bang::crc(v) << '\n' ;

    auto pad = to_ull(v) ;
    std::cout << pad << '\n' ;
}

static void rom(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    RpiExt::Bang::Stack stack(0x100) ;
    Bang::Record record ;
    RpiExt::Bang scheduler(rpi) ;

    auto script = Bang(rpi,pin).readRom(&record,&stack) ;
    auto success = scheduler.execute(script) ;
    if (!success)
	std::cerr << "timing failed\n" ;

    // debugging
    auto v = Bang::assemble(record.buffer,64,1u<<pin.value()) ; 
    std::cout << v << '\n' ;
    v = Bang::assemble(record.buffer,56,1u<<pin.value()) ; 
    std::cout << std::hex << (unsigned)Bang::crc(v) << '\n' ;

    auto code = to_ull(v) ;
    std::cout << code << '\n' ;
}

#include <Rpi/Timer.h>
#include <sys/time.h>
#include <sys/resource.h>
// The _GNU_SOURCE feature test macro must be defined (before including
// any header file) in order to obtain the definition of this constant
// from <sys/resource.h>.
// RUSAGE_THREAD is Linux-specific as all the fields used here
//#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <Posix/base.h>
static void test(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto us = Ui::strto<unsigned>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Timer timer(rpi) ;
    
    clockid_t id ;
    auto result = clock_getcpuclockid(0,&id);
    assert(result == 0) ;

    struct timespec ts0 ;
    result = clock_gettime(id,&ts0) ;
    assert(result == 0) ;
    
    struct rusage u0 ;
    result = getrusage(RUSAGE_THREAD,&u0) ;
    assert(result == 0) ;
    
    auto t0 = timer.cLo() ;
    while (timer.cLo() - t0 < us)
	Posix::nanosleep(1e+8) ;

    struct timespec ts1 ;
    result = clock_gettime(id,&ts1) ;
    assert(result == 0) ;

    struct rusage u1 ;
    result = getrusage(RUSAGE_THREAD,&u1) ;
    assert(result == 0) ;

    std::cout << "clock: "
	      << ts0.tv_sec << ' ' << ts0.tv_nsec  << ' ' 
	      << ts1.tv_sec << ' ' << ts1.tv_nsec  << '\n' ;

    std::cout << "  voluntary: " << u0.ru_nvcsw    << ' ' << u1.ru_nvcsw    << '\n' ;
    std::cout << "involuntary: " << u0.ru_nivcsw   << ' ' << u1.ru_nivcsw   << '\n' ;
    std::cout << "    signals: " << u0.ru_nsignals << ' ' << u1.ru_nsignals << '\n' ;
}

void Console::Device::Ds18x20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: OPTION PIN\n"
		  << '\n'
		  << "rom = read 64-bit ROM code\n"
		  << "pad = read 72-bit scratch pad\n"
	    ;
	return ;
    }

    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "convert") convert(rpi,argL) ;
    else if (arg == "rom") rom(rpi,argL) ;
    else if (arg == "pad") pad(rpi,argL) ;
    
    else if (arg == "test") test(rpi,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
