// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang/Bang.h>
#include <Device/Ds18x20/Bang/OneWire/Error.h>
#include <Device/Ds18x20/Bang/OneWire/Addressing.h>
#include <Posix/base.h>
#include <Ui/strto.h>
#include <cstring> // memset
#include <iomanip>

// [todo] command line options: timing and ARM counter frequency

using namespace Device::Ds18x20::Bang ;

using Error = OneWire::Error ;

using Master = OneWire::Master ;

using Addressing = OneWire::Addressing ;

using Address = Addressing::Address ; // todo OneWire::Address

// ----

template<size_t N> static void print(std::bitset<N> const &set)
{
    std::ostringstream os ;
    unsigned v = 0 ;
    for (size_t i=0 ; i<N ; ++i)
    {
	v >>= 1 ;
	if (set[i])
	    v |= 0x80 ;
	
	if (((i % 8) == 7) || (i == N-1))
	{
	    os << std::hex << v/16 << v%16 << ' ' ;
	    v = 0 ;
	}
    } // [todo] bitset to array to hex-decimal
    
    auto crc = 0 == Bang::crc(set) ;
    auto string = set.to_string() ;
    std::reverse(string.begin(),string.end()) ;
    std::cout
	<< os.str() << string << ' '
	<< (crc ? "crc:ok" : "crc:failure") << '\n' ;
}

// ----

static boost::optional<Address>
address(Master *master,bool retry)
{
  Retry: try { return Addressing(master).get() ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static boost::optional<Address>
first(Master *master,bool retry)
{
  Retry: try { return Addressing(master).first() ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static boost::optional<Address>
next(Master *master,Address const &address,bool retry)
{
  Retry: try { return Addressing(master).next(address) ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

// ----

static void convert(Master *master,bool retry)
{
  Retry: try { Bang(master).convert() ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static bool isBusy(Master *master,bool retry)
{
  Retry: try { return Bang(master).isBusy() ; }
    
    catch(Error &error)    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static bool isPowered(Master *master,bool retry)
{
  Retry: try { return Bang(master).isPowered() ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static Bang::Pad readPad(Master *master,bool retry)
{
  Retry: try { return Bang(master).readPad() ; }
    
    catch(Error &error)
    {
	if (retry && error.type() == Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

// ----

static void convert(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto wait = !argL->pop_if("-n") ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;

    convert(&master,retry) ;

    if (wait)
    {
	unsigned count = 1 ;
	
	auto busy = isBusy(&master,retry) ;
	while (busy)
	{
	    busy = isBusy(&master,retry) ;
	    ++count ;
	}
	// [todo] measure and display the time it takes to finish
	// (this piece of information might be quite interesting)
	
	// this does only word on a single drop bus
	auto pad = readPad(&master,retry) ;
	print(pad) ;
	auto temp = (pad & Bang::Pad(0xffff)).to_ullong() ;
	auto mode = ((pad >> 37) & Bang::Pad(0x3)).to_ullong() ;
	auto div = 2u << mode ;
	std::cout << static_cast<double>(temp) / div << '\n' ;
	// todo: record time
    }
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;

    auto pad = readPad(&master,retry) ;
    print(pad) ;
}

static void power(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;
    
    auto powered = isPowered(&master,retry) ;
    std::cout << powered << '\n' ;
}

// ----

static void address(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;
    
    auto address = ::address(&master,retry) ;
    if (address)
    {
	print(*address) ;
    }
    else
    {
	std::cout << "no device present\n" ;
    }
}

static void search(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;

    auto address = first(&master,retry) ;
    if (!address)
    {
	std::cout << "no device present\n" ;
    }
    else while (address)
	 {
	     print(*address) ;
	     address = next(&master,*address,retry) ;
	 }
}

// ----

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

// ----

void Console::Device::Ds18x20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	{ "address" , address },
	{ "convert" , convert },
	{ "help"    ,    help },
	{ "pad"     ,     pad },
	{ "power"   ,   power },
	{ "search"  ,  search },
    } ;
    argL->pop(map)(rpi,argL) ;
}
