// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Ui/strto.h>
#include <cstring> // memset

// [todo] command line options: timing and ARM counter frequency

static void pack(bool const from[],size_t nbits,char to[])
{
    memset(to,0x0,(nbits+7)/8) ;
    for (decltype(nbits) i=0 ; i<nbits ; ++i)
    {
	if (from[i])
	    to[i/8] |= static_cast<char>(1 << (i % 8)) ;
    }
}

static std::vector<bool> to_bitStream(char const buffer[],size_t n)
{
    std::vector<bool> v ; v.reserve(n*8) ;
    for (decltype(n) i=0 ; i<n ; ++i)
	for (auto j=0 ; j<8 ; ++j)
	    v.push_back(0 != (buffer[i] & (1u << j))) ;
    return v ;
}

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
    auto wait = !argL->pop_if("-n") ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    RpiExt::BangIo io(rpi) ;

    try
    {
	Bang(rpi,pin).convert(&io) ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "start conversion error:" << e.what() << '\n' ;
	exit(1) ;
    }

    if (wait)
    {
	unsigned count = 1 ;
      Proceed: ;
	try
	{
	    auto busy = Bang(rpi,pin).isBusy(&io) ;
	    while (busy)
	    {
		busy = Bang(rpi,pin).isBusy(&io) ;
		++count ;
	    }
	}
	catch (Bang::Error &e)
	{
	    if (0 == strcmp(e.what(),"reset"))
	    {
		std::cerr << "got suspended while setting the bus low\n" ;
		exit(1) ;
	    }
	    goto Proceed ;
	}
	
	bool rx[72] ;
	try
	{
	    Bang(rpi,pin).readPad(&io,&rx) ;
	}
	catch (Bang::Error &e)
	{
	    std::cerr << "read scratch-pad error:" << e.what() << '\n' ;
	    exit(1) ;
	}
	char buffer[9] ; pack(rx,72,buffer) ;
	auto bs = to_bitStream(buffer,sizeof(buffer)) ;
	if (0 != Bang::crc(bs))
	{
	    std::cerr << "read scratch-pad crc error\n" ;
	    exit(1) ;
	}
	auto pad = to_ull(bs) ;
	auto temp = static_cast<int16_t>(pad & 0xffff) ;
	auto mode = static_cast<unsigned>((pad >> 37) & 0x3) ;
	auto div = 2u << mode ;
	std::cout << static_cast<double>(temp) / div << '\n' ;
    }
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    bool rx[72] ;
    RpiExt::BangIo io(rpi) ;

    try
    {
	Bang(rpi,pin).readPad(&io,&rx) ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	exit(1) ;
    }
    
    char buffer[9] ; pack(rx,72,buffer) ;
    // debugging
    auto v = to_bitStream(buffer,sizeof(buffer)) ; 
    std::cout << v << '\n' ;
    v = to_bitStream(buffer,sizeof(buffer)-1) ; 
    std::cout << std::hex << (unsigned)Bang::crc(v) << '\n' ;

    auto pad = to_ull(v) ;
    std::cout << pad << '\n' ;
}

static void rom(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    bool rx[64] ;
    RpiExt::BangIo io(rpi) ;

    try
    {
	Bang(rpi,pin).readRom(&io,&rx) ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	exit(1) ;
    }
	
    char buffer[8] ; pack(rx,64,buffer) ;
    
    // debugging
    auto v = to_bitStream(buffer,sizeof(buffer)) ; 
    std::cout << v << '\n' ;
    v = to_bitStream(buffer,sizeof(buffer)-1) ; 
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
