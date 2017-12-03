// BSD 2-Clause License, see github.com/ma16/rpio

// see README.md for details

#include "invoke.h"
#include <Neat/cast.h>
#include <Rpi/Pin.h>
#include <Rpi/Register.h>
#include <Ui/strto.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>

using Clock = std::chrono::steady_clock ;

using Duration = std::chrono::duration<double> ;

// save samples to RAM and write thereafter to file
static void buffer(uint32_t volatile *port,size_t nsamples,Ui::ArgL *argL) 
{
    auto fname = argL->pop() ;
    argL->finalize() ;
    std::ofstream os(fname) ;
    if (!os)
	throw std::runtime_error("cannot open:" + fname) ;
    auto buffer = new uint32_t [nsamples] ;
    // ...unscoped on purpose (keeps it simple)
    std::fill(buffer+0,buffer+nsamples,0u) ;
    // ...resolve zero pages (may work/help)
    buffer[0] = 0 ;
    // ...make cpu/bus aware of the top of the buffer (may work/help)
    auto t0 = Clock::now() ;
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
    {
	buffer[i] = (*port) ;
    }
    auto dt = Duration(Clock::now()-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << nsamples/dt << "/s\n"
	      << "writing file..." << std::flush ;
    auto p = reinterpret_cast<char*>(buffer) ;
    auto nbytes = nsamples * sizeof(buffer[0]) ;
    os.write(p,Neat::to_signed(nbytes)) ;
    std::cout << " done\n" ;
}

// determine maximum sample rate
static void dry(uint32_t volatile *port,size_t nsamples,Ui::ArgL *argL) 
{
    argL->finalize() ;
    auto t0 = Clock::now() ;
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
    {
	(*port) ;
    }
    auto dt = Duration(Clock::now()-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << nsamples/dt << "/s\n" ;
}

// measure duty cycle
static void duty(uint32_t volatile *port,size_t nsamples,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    auto mask = 1u << pin.value() ;
    decltype(nsamples) nhi = 0 ; 
    auto t0 = Clock::now() ;
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
    {
	auto hi = 0 != ((*port) & mask) ;
	if (hi)
	    ++nhi ;
    }
    auto dt = Duration(Clock::now()-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << nsamples/dt << "/s " 
	      << "d=" << 1.0 * nhi / nsamples << "\n" ;
}

// measure frequency
static void frequency(uint32_t volatile *port,size_t nsamples,Ui::ArgL *argL) 
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    auto mask = 1u << pin.value() ;
    decltype(nsamples) nchanges = 0 ;
    auto level = (*port) & mask ;
    auto t0 = Clock::now() ;
    for (decltype(nsamples) i=0 ; i<nsamples ; ++i)
    {
	auto next = (*port) & mask ;
	if (next != level)
	{
	    level = next ;
	    ++nchanges ;
	}
    }
    auto dt = Duration(Clock::now()-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    
    std::cout << "r=" << nsamples/dt << "/s " 
	      << "f=" << nchanges/dt/2 << "/s\n" ;
}

// similar to "buffer", but store records: record=(level,repetitions)
struct Record
{
    uint32_t value ; // GPIO pin 0-31 input levels
    uint32_t nreps ; // repetitions
} ;

static size_t pool(uint32_t volatile *port,
		   uint32_t           mask,
		   size_t         nsamples,
		   Record          *buffer,
		   size_t         capacity) // max number of records in buffer
{
    if (nsamples == 0) 
	return 0 ;
    decltype(capacity) index = 0 ; // index into buffer
    auto value = (*port) & mask ;
    decltype(nsamples) nreps = 1 ;
    for (decltype(nsamples) i=1 ; i<nsamples ; ++i)
    {
	auto next = (*port) & mask ;
	if (index == capacity)
	    continue ; // buffer full
	constexpr auto max = std::numeric_limits<decltype(nreps)>::max() ;
	if (next == value)
	{
	    if (nreps < max)
	    {
		++nreps ;  
		continue ;
	    }
	    // else: fall thru and save record
	}
	buffer[index].value = value ;
	buffer[index].nreps = nreps ; 
	++index ; 
	value = next ;
	nreps = 1 ;
    }
    if (index < capacity)
    {
	buffer[index].value = value ;
	buffer[index].nreps = nreps ; 
	++index ;
    }
    return index ;
}

static void pool(uint32_t volatile *port,size_t nsamples,Ui::ArgL *argL) 
{
    auto fname = argL->pop() ;
    auto capacity = Ui::strto<uint32_t>(argL->option("-c",std::to_string(nsamples))) ;
    auto mask = Ui::strto<uint32_t>(argL->option("-m","0xffffffff")) ;
    argL->finalize() ;
    std::ofstream os(fname) ;
    if (!os)
	throw std::runtime_error("cannot open:" + fname) ;
    auto buffer = new Record [capacity] ;
    // ...unscoped on purpose (keeps it simple)
    std::fill(buffer+0,buffer+capacity,Record{0,0}) ;
    // ...resolve zero pages (may work/help)
    buffer[0] = Record{0,0} ;
    // ...make cpu/bus aware of the top of the buffer (may work/help)
    auto t0 = Clock::now() ;
    auto nrecords = pool(port,mask,nsamples,buffer,capacity) ;
    auto dt = Duration(Clock::now()-t0).count() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << "r=" << nsamples/dt << "/s "
	      << "n=" << nrecords << "\n"
	      << "writing file..." << std::flush ;
    auto p = reinterpret_cast<char*>(buffer) ;
    auto nbytes = nrecords * sizeof(buffer[0]) ;
    os.write(p,Neat::to_signed(nbytes)) ;
    std::cout << " done\n" ;
}

// --------------------------------------------------------------------

void Console::Sample::invoke_level(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout
	    << "arguments: [-r SOURCE] NSAMPLES ...\n"
	    << '\n'
	    << "SOURCE: peripheral register to read from, e.g.\n"
	    << "  -   0x3004 : static  1-µs-increment free running counter\n"
	    << "  -   0xb420 : up to 2.5-ns-increment free running counter\n"
	    << "  - 0x200034 : GPIO pin  0..31 (default)\n"
	    << "  - 0x200038 : GPIO pin 32..53\n"
	    << '\n'
	    << "... buffer FILE\n"
	    << "  Save samples to RAM and write buffer to FILE when finished.\n"
	    << '\n'
	    << "... dry\n"
	    << "  Read samples. Ignore their values. Useful as benchmark.\n"
	    << '\n'
	    << "... duty BIT=0..31\n"
	    << "  Count the number of High and Low levels.\n"
	    << '\n'
	    << "... frequency BIT=0..31\n"
	    << "  Count the number of transitions from High to Low level.\n"
	    << '\n'
	    << "... pool FILE [-c CAPACITY] [-m MASK]\n"
	    << "  Save (VALUE,n) records to RAM; where VALUE := SAMPLE & MASK and\n"
	    << "  where (n) holds the number of repetitions. The records are written\n"
	    << "  to FILE when sampling has finished. CAPACITY defines the maximum\n"
	    << "  number of records. CAPACITY defaults to NSAMPLES, MASK to 0xffffffff.\n"
	    ;
	return ;
    }
    
    auto source = rpi->at<Rpi::Register::Gpio::Input::Bank0>().value() ;
    if (argL->pop_if("-r"))
    {
	auto i = Ui::strto<uint32_t>(argL->pop()) ;
	source = & rpi->at(i) ;
    }
    
    auto nsamples = Ui::strto<size_t>(argL->pop()) ;
    
    std::map<std::string,void(*)(uint32_t volatile*,size_t,Ui::ArgL*)> map =
    {
	{ "buffer"    ,    buffer },
	{ "dry"       ,       dry },
	{ "duty"      ,      duty },
	{ "frequency" , frequency },
	{ "pool"      ,      pool },
    } ;
    argL->pop(map)(source,nsamples,argL) ;
}
