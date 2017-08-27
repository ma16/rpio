// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Ui/strto.h>

static void display(uint32_t const *buffer,uint32_t mask,size_t nwords)
{
    for (auto i=0u ; i<nwords/8 ; ++i)
    {
	unsigned code = 0 ;
	for (auto j=0u ; j<8u ; ++j)
	{
	    code <<= 1 ;
	    if (buffer[8*i+7-j] & mask)
		code |= 1u ;
	}
	std::cout << std::hex << code << ' ' ;
    }
    std::cout << '\n' ;
    {
	for (auto i=0u ; i<nwords ; ++i)
	{
	    auto bit = 0 != (buffer[i] & mask) ;
	    std::cout << bit ;
	    if ((i % 8) == 7) std::cout << ' ' ;
	    else if ((i % 4) == 3) std::cout << ':' ;
	}
	std::cout << '\n' ;
    }
}

static void doit(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    Bang::Record record ;
    RpiExt::Bang scheduler(rpi) ;

    auto script = Bang(rpi,pin).makeScript(&record) ;
    scheduler.execute(script) ;

    display(record.buffer,1u<<pin.value(),64) ;
    std::cout << std::hex << (unsigned)Bang::crc(record.buffer,1u<<pin.value(),56) << '\n' ;

    auto script2 = Bang(rpi,pin).makeScript2(&record) ;
    scheduler.execute(script2) ;

    display(record.buffer,1u<<pin.value(),72) ;
    std::cout << std::hex << (unsigned)Bang::crc(record.buffer,1u<<pin.value(),64) << '\n' ;
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
	std::cout << "arguments: ... \n" ;
	return ;
    }

    std::string arg = argL->pop() ;
    if (false) ;
  
    else if (arg == "doit") doit(rpi,argL) ;
    else if (arg == "test") test(rpi,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
