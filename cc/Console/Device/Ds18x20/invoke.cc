// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Posix/base.h>
#include <Ui/strto.h>
#include <cstring> // memset
#include <iomanip>

// [todo] command line options: timing and ARM counter frequency

template<size_t N> static void print(std::bitset<N> const &set)
{
    using Bang = Device::Ds18x20::Bang ;
    std::ostringstream os ;
    unsigned v = 0 ;
    for (auto i=N ; i>0 ; )
    {
	--i ;
	v |= set[i] ;
	
	if ((i % 8) == 0)
	{
	    os << std::hex << std::setfill('0') << std::setw(2) << v << ' ' ;
	    v = 0 ;
	}
	else
	{
	    v <<= 1 ;
	}
    }
    
    auto crc = 0 == Bang::crc(set) ;
    auto string = set.to_string() ;
    std::reverse(string.begin(),string.end()) ;
    std::cout
	<< os.str()
	<< string << ' '
	<< (crc ? "crc:ok" : "crc:failure") << '\n' ;
}

static void convert(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto wait = !argL->pop_if("-n") ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;
    Bang bang(rpi,pin) ;

  Retry: ;
    
    try
    {
	bang.convert() ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "start conversion error:" << e.what() << '\n' ;
	goto Retry ;
    }

    if (wait)
    {
	unsigned count = 1 ;
	
      Proceed: ;
	
	try
	{
	    auto busy = bang.isBusy() ;
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
		std::cerr << "got suspended\n" ;
		exit(1) ;
	    }
	    goto Proceed ;
	}
	// [todo] measure and display the time it takes to finish
	// (this piece of information might be quite interesting)
	
	try
	{
	    // this does only word on a single drop bus
	    auto pad = bang.readPad() ;
	    print(pad) ;
	    auto temp = (pad & Bang::Pad(0xffff)).to_ullong() ;
	    auto mode = ((pad >> 37) & Bang::Pad(0x3)).to_ullong() ;
	    auto div = 2u << mode ;
	    std::cout << static_cast<double>(temp) / div << '\n' ;
	    // todo: record time
	}
	catch (Bang::Error &e)
	{
	    std::cerr << "read scratch-pad error:" << e.what() << '\n' ;
	    exit(1) ;
	    // todo: repeat
	}
    }
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    using Bang = Device::Ds18x20::Bang ;

  Retry:
    try
    {
	auto pad = Bang(rpi,pin).readPad() ;
	print(pad) ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	goto Retry ;
    }
}

static void power(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
    using Bang = Device::Ds18x20::Bang ;
    Bang bang(rpi,pin) ;

  Retry:
    
    try
    {
	auto powered = bang.isPowered() ;
	std::cout << powered << '\n' ;
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	goto Retry ;
    }
}

static void rom(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    using Bang = Device::Ds18x20::Bang ;
  
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    argL->finalize() ;
    
  Retry:
    try
    {
	auto address = Bang(rpi,pin).address() ;
	if (address)
	{
	    print(*address) ;
	}
	else
	{
	    std::cout << "no device present\n" ;
	}
    }
    catch (Bang::Error &e)
    {
	std::cerr << "error:" << e.what() << '\n' ;
	goto Retry ;
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

static void help(Rpi::Peripheral*,Ui::ArgL*)
{
    std::cout
	<< "arguments: OPTION PIN\n"
	<< '\n'
	<< "rom = read 64-bit ROM code\n"
	<< "pad = read 72-bit scratch pad\n"
	<< "power = echo 1 if powered; 0 if in parasite power mode\n"
	<< "... todo ...\n"
	;
}

void Console::Device::Ds18x20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "convert" , convert },
	{ "help"    ,    help },
	{ "rom"     ,     rom },
	{ "pad"     ,     pad },
	{ "power"   ,   power },
	{ "search"  ,  search },
    } ;
    argL->pop(map)(rpi,argL) ;
}
