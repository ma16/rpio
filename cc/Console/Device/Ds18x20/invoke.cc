// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18x20/Bang.h>
#include <Posix/base.h>
#include <Ui/strto.h>
#include <cstring> // memset
#include <iomanip>

// [todo] command line options: timing and ARM counter frequency

using Bang = Device::Ds18x20::Bang ;

// ----

template<size_t N> static void print(std::bitset<N> const &set)
{
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

// ----

static boost::optional<Bang::Address>
address(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { return bang->address() ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static boost::optional<Bang::Address>
first(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { return bang->first() ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static boost::optional<Bang::Address>
next(Device::Ds18x20::Bang *bang,Bang::Address const &address,bool retry)
{
  Retry: try { return bang->next(address) ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

// ----

static void convert(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { bang->convert() ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static bool isBusy(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { return bang->isBusy() ; }
    
    catch(Bang::Error &error)    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static bool isPowered(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { return bang->isPowered() ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
	    goto Retry ;
	throw ;
    }
}

static Bang::Pad readPad(Device::Ds18x20::Bang *bang,bool retry)
{
  Retry: try { return bang->readPad() ; }
    
    catch(Bang::Error &error)
    {
	if (retry && error.type() == Bang::Error::Type::Retry)
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
    
    Bang bang(rpi,pin) ;

    convert(&bang,retry) ;

    if (wait)
    {
	unsigned count = 1 ;
	
	auto busy = isBusy(&bang,retry) ;
	while (busy)
	{
	    busy = isBusy(&bang,retry) ;
	    ++count ;
	}
	// [todo] measure and display the time it takes to finish
	// (this piece of information might be quite interesting)
	
	// this does only word on a single drop bus
	auto pad = readPad(&bang,retry) ;
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
    Bang bang(rpi,pin) ;

    auto pad = readPad(&bang,retry) ;
    print(pad) ;
}

static void power(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Bang bang(rpi,pin) ;
    
    auto powered = isPowered(&bang,retry) ;
    std::cout << powered << '\n' ;
}

// ----

static void address(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Bang bang(rpi,pin) ;
    
    auto address = ::address(&bang,retry) ;
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
    Bang bang(rpi,pin) ;

    auto address = first(&bang,retry) ;
    if (!address)
    {
	std::cout << "no device present\n" ;
    }
    else while (address)
	 {
	     print(*address) ;
	     address = next(&bang,*address,retry) ;
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
