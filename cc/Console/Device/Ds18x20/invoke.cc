// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Posix/base.h>
#include <Ui/strto.h>
#include <cstring> // memset

// [todo] command line options: timing and ARM counter frequency

template<size_t N> static void print(std::bitset<N> const &set)
{
    using Bang = Device::Ds18x20::Bang ;
    auto crc = 0 == Bang::crc(set) ;
    auto string = set.to_string() ;
    std::reverse(string.begin(),string.end()) ;
    std::cout
	<< std::hex << set.to_ullong() << ' '
	<< string << ' '
	<< (crc ? "crc:ok" : "crc:failure") << '\n' ;
}

static void convert(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto wait = !argL->pop_if("-n") ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    try
    {
	Bang(rpi,pin).convert() ;
	// [todo] retry
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
	    auto busy = Bang(rpi,pin).isBusy() ;
	    while (busy)
	    {
		busy = Bang(rpi,pin).isBusy() ;
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
	// [todo] measure and display the time it takes to finish
	// (this piece of information might be quite interesting)
	
	try
	{
	    // this does only word on a single drop bus
	    auto pad = Bang(rpi,pin).readPad() ;
	    print(pad) ;
	    auto ull = pad.to_ullong() ;
	    auto temp = static_cast<int16_t>(ull & 0xffff) ;
	    auto mode = static_cast<unsigned>((ull >> 37) & 0x3) ;
	    auto div = 2u << mode ;
	    std::cout << static_cast<double>(temp) / div << '\n' ;
	}
	catch (Bang::Error &e)
	{
	    std::cerr << "read scratch-pad error:" << e.what() << '\n' ;
	    exit(1) ;
	}
    }
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    
    try
    {
	auto pad = Bang(rpi,pin).readPad() ;
	print(pad) ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	exit(1) ;
    }
}

static void rom(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;

    try
    {
	auto address = Bang(rpi,pin).address() ;
	if (address)
	    print(*address) ;
	else
	    std::cout << "no device present\n" ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	exit(1) ;
    }
}

static void search(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    Bang bang(rpi,pin) ;

    auto first = [&bang]
    {
      Retry: ;
      try { return bang.first() ; }
      catch (Bang::Error &e)
      {
	  std::cerr << "error:" << e.what() << '\n' ;
	  goto Retry ;
      }
    } ;
    
    auto address = first() ;

    auto next = [&bang](Bang::Address const &address)
    {
      Retry: ;
      try { return bang.next(address) ; }
      catch (Bang::Error &e)
      {
	  std::cerr << "error:" << e.what() << '\n' ;
	  goto Retry ;
      }
    } ;

    while (address)
    {
	print(*address) ;
	address = next(*address) ;
    }
    
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
    else if (arg == "search") search(rpi,argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
