// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Device/Ds18b20/Bang.h>
#include <Protocol/OneWire/Bang/Addressing.h>
#include <Protocol/OneWire/Bang/crc.h>
#include <Protocol/OneWire/Bang/Error.h>
#include <Ui/strto.h>
#include <chrono>
#include <iomanip>

using Ds18b20 = Device::Ds18b20::Bang ;

using Error = Protocol::OneWire::Bang::Error ;

using Master = Protocol::OneWire::Bang::Master ;

using Addressing = Protocol::OneWire::Bang::Addressing ;

using Address = Protocol::OneWire::Bang::Address ; 

// ----[ 1-wire retry-wrappers ]---------------------------------------

static void handle(Error const &error,bool debug,bool retry)
{
    if (error.type() != Error::Type::Retry)
	throw ;
    if (!retry)
	throw ;
    if (debug)
	std::cout << error.what() << '\n' ;
}

static boost::optional<Address>
rom(Master *master,bool debug,bool retry)
{
  Retry:
    try { return Addressing(master).rom() ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static boost::optional<Address>
first(Master *master,bool alarm,bool debug,bool retry)
{
  Retry:
    try { return Addressing(master).first(alarm) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static boost::optional<Address>
next(Master *master,Address const &address,bool alarm,bool debug,bool retry)
{
  Retry:
    try { return Addressing(master).next(address,alarm) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

// ----[ DS18B20 retry-wrappers ]--------------------------------------

static void convert(
    Master *master,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { Ds18b20(master).convert(address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static bool isBusy(Master *master,bool debug,bool retry)
{
  Retry:
    try { return Ds18b20(master).isBusy() ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static bool isPowered(
    Master *master,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { return Ds18b20(master).isPowered(address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static Ds18b20::Pad readPad(
    Master *master,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { return Ds18b20(master).readPad(address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static void restore(
    Master *master,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { Ds18b20(master).restore(address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static void save(
    Master *master,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { Ds18b20(master).save(address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

static void write(
    Master *master,
    Ds18b20::Mod mod,
    boost::optional<Address> const &address,
    bool debug,
    bool retry)
{
  Retry:
    try { return Ds18b20(master).write(mod,address) ; }
    catch(Error &error) { handle(error,debug,retry) ; goto Retry ; }
}

// ----[ format bitset ]-----------------------------------------------

template<size_t N>
static std::array<uint8_t,(N+7)/8> toByteA(std::bitset<N> const &set)
{
    std::array<uint8_t,(N+7)/8> byteA ;
    for (size_t i=0 ; 8*i<N ; ++i)
    {
	uint8_t b = 0 ;
	for (size_t j=0 ; 8*i+j<N ; ++j)
	    b = static_cast<uint8_t>(b | (set[8*i+j] << j)) ;
	byteA[i] = b ;
    }
    return byteA ;
}

template<size_t N>
static std::bitset<N> to_bitset(std::array<uint8_t,(N+7)/8> const& byteA)
{
    std::bitset<N> set ;
    for (size_t i=0 ; 8*i<N ; ++i)
    {
	uint8_t b = byteA[i] ;
	for (size_t j=0 ; 8*i+j<N ; ++j)
	    set[8*i+j] = 0 != (b & (1u<<j)) ;
    }
    return set ;
}

// ----

template<size_t N>
static std::array<uint8_t,N> toByteA(std::string const &hex)
{
    static_assert(N>0,"") ;
    std::array<uint8_t,N> byteA ;
    if (3*N-1 != hex.length()) 
	throw std::runtime_error("invalid size of hex-string") ;
    auto xlat = [] (char c) {
	if ('0' <= c && c <= '9') return c - '0' ;
	if ('A' <= c && c <= 'F') return c - 'A' + 10 ;
	if ('a' <= c && c <= 'f') return c - 'a' + 10 ;
	throw std::runtime_error("invalid hex-digit") ;
    } ;
    auto i=0u,j=0u ;
    while (true)
    {
	auto byte = 16 * xlat(hex[i]) + xlat(hex[i+1]) ;
	byteA[j] = static_cast<uint8_t>(byte) ;
	if (++j == N)
	    break ;
	if (':' != hex[i+2])
	    throw std::runtime_error("separator expected in hex-string") ;
	i += 3 ;
    }
    return byteA ;
}    

template<size_t N>
static std::string toHexStr(std::array<uint8_t,N> const &byteA)
{
    std::ostringstream os ;
    os << std::hex ;
    auto i = byteA.cbegin() ;
    while (i != byteA.cend())
    {
	os << (*i)/16 << (*i)%16 ;
	while (++i != byteA.cend())
	    os << ':' << (*i)/16 << (*i)%16 ;
    }
    return os.str() ;
}    

// ----

template<size_t N>
static std::string toStr(std::bitset<N> const &set,bool debug)
{
    std::ostringstream os ;
    auto byteA = toByteA(set) ;
    os << toHexStr(byteA) ;
    auto crc = 0 == Protocol::OneWire::Bang::crc(set) ;
    os << ' ' << (crc ? "crc:ok" : "crc:failure") ;
    if (debug)
    {
	auto string = set.to_string() ;
	std::reverse(string.begin(),string.end()) ;
	os << ' ' << string ;
    }
    return os.str() ;
}

// ----[ 1-wire console commands ]-------------------------------------

static void rom(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;
    auto address = rom(&master,debug,retry) ;
    if (address)
    {
	std::cout << toStr(*address,debug) << '\n' ;
    }
    else
    {
	std::cout << "no device present\n" ;
    }
}

static void search(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto alarm = argL->pop_if("-a") ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;
    auto address = first(&master,alarm,debug,retry) ;
    if (!address)
    {
	std::cout << "no device present\n" ;
    }
    else
    {
	while (address)
	 {
	     std::cout << toStr(*address,debug) << '\n' ;
	     address = next(&master,*address,alarm,debug,retry) ;
	 }
    }
}

// ----[ DS18B20 console commands ]------------------------------------

static boost::optional<Address> optAddress(Ui::ArgL *argL)
{
    auto exists = argL->pop_if("-a") ;
    if (!exists)
	return boost::none ;
    auto hex = argL->pop() ;
    auto byteA = toByteA<8>(hex) ;
    auto set = to_bitset<64>(byteA) ;
    return set ;
}

static void convert(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    auto wait = argL->pop_if("-w") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;

    auto t0 = std::chrono::steady_clock::now() ;
    convert(&master,address,debug,retry) ;
    if (!wait)
	return ;

    auto busy = isBusy(&master,debug,retry) ;
    while (busy)
	busy = isBusy(&master,debug,retry) ;
    auto t1 = std::chrono::steady_clock::now() ;
    auto dt = std::chrono::duration<double>(t1-t0).count() ;

    // reading doesn't work on broadcast and multi-drop bus
    auto pad = readPad(&master,address,debug,retry) ;
#if 0    
    auto temp = (pad & Ds18b20::Pad(0xffff)).to_ullong() ;
    auto mode = ((pad >> 37) & Ds18b20::Pad(0x3)).to_ullong() ;
    auto div = 2u << mode ;
    std::cout << static_cast<double>(temp) / div << '\n' ;
#endif    

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << toStr(pad,debug) << " (" << dt << "s)\n" ;
}

static void pad(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;
    auto pad = readPad(&master,address,debug,retry) ;
    std::cout << toStr(pad,debug) << '\n' ;
}

static void power(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    argL->finalize() ;
    Master master(rpi,pin) ;
    auto powered = isPowered(&master,address,debug,retry) ;
    std::cout << powered << '\n' ;
}

static void restore(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    auto wait = argL->pop_if("-w") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;

    auto t0 = std::chrono::steady_clock::now() ;
    restore(&master,address,debug,retry) ;
    if (!wait)
	return ;
    auto busy = isBusy(&master,debug,retry) ;
    while (busy)
	busy = isBusy(&master,debug,retry) ;
    auto t1 = std::chrono::steady_clock::now() ;
    auto dt = std::chrono::duration<double>(t1-t0).count() ;

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << dt << "s\n" ;
}

static void save(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    auto wait = argL->pop_if("-w") ;
    argL->finalize() ;
    
    Master master(rpi,pin) ;

    auto t0 = std::chrono::steady_clock::now() ;
    save(&master,address,debug,retry) ;
    if (!wait)
	return ;
    auto busy = isBusy(&master,debug,retry) ;
    while (busy)
	busy = isBusy(&master,debug,retry) ;
    auto t1 = std::chrono::steady_clock::now() ;
    auto dt = std::chrono::duration<double>(t1-t0).count() ;

    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << dt << "s\n" ;
}

static void write(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto pin = Ui::strto(argL->pop(),Rpi::Pin()) ;
    auto address = optAddress(argL) ;
    auto debug = argL->pop_if("-d") ;
    auto retry = argL->pop_if("-r") ;
    auto th = Ui::strto<uint8_t>(argL->pop()) ;
    auto tl = Ui::strto<uint8_t>(argL->pop()) ;
    auto cf = Ui::strto<uint8_t>(argL->pop()) ;
    argL->finalize() ;
    Master master(rpi,pin) ;
    auto mod = static_cast<uint32_t>((th<<0) | (tl<<8) | (cf<<16)) ;
    write(&master,Ds18b20::Mod(mod),address,debug,retry) ;
}

// ----

static void help(Rpi::Peripheral*,Ui::ArgL*)
{
    std::cout
	<< "arguments: COMMAND...\n"
	<< '\n'
	<< "convert PIN ADDRESS [-d] [-r] [-w]\n"
	<< "pad     PIN ADDRESS [-d] [-r]\n"
	<< "power   PIN ADDRESS [-d] [-r]\n"
	<< "restore PIN ADDRESS [-d] [-r] [-w]\n"
	<< "rom     PIN         [-d] [-r]\n"
	<< "save    PIN ADDRESS [-d] [-r] [-w]\n"
	<< "search  PIN [-a]    [-d] [-r]\n" 
	<< "write   PIN ADDRESS [-d] [-r] TH TL CF\n"
	<< '\n'
	<< "convert: Convert-T Function\n"
	<< "    [-w] wait for completion and then issue 'pad'\n"
	<< '\n'
	<< "pad: Read Scratch-Pad Function\n"
	<< '\n'
	<< "power: Read Power-Supply Function\n"
	<< "    echo 1 if Vdd powered\n"
	<< "         0 if in parasite power-mode\n"
	<< '\n'
	<< "restore: Recall E2 Function\n"
	<< "    [-w] wait for completion\n"
	<< '\n'
	<< "rom: Read ROM command\n"
	<< '\n'
	<< "save: Copy Scratch-Pad Function\n"
	<< "    [-w] wait for completion\n"
	<< '\n'
	<< "search: Alarm/Search-ROM command\n"
	<< "    Issues a series of commands to displays all attached devices\n"
	<< "    [-a] do Alarm-Search command instead of Seach-ROM\n"
	<< '\n'
	<< "write: Write Scratch-Pad Function\n"
	<< "    TH: high temperature threshold (uint8)\n"
	<< "    TL:  low temperature threshold (uint8)\n"
	<< "    CF: configuration register (uint8)\n"
	<< '\n'
	<< "Options:\n"
	<< "[-d] display debug messages\n"
	<< "[-r] retry if timing wasn't met\n"
	<< '\n'
	<< "ADDRESS: [-a XX:XX...]\n"
	<< "    do Match-ROM command instead of Skip-ROM\n"
	<< "    e.g. -a 28:ff:40:16:c2:16:03:28\n"
	;
}

// ----

void Console::Device::Ds18b20::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    // [todo] command line options: timing and ARM counter frequency

    std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
    {
	// diagnostic help message:
	{ "help"    ,    help },
	
	// generic 1-Wire ROM commands:
	{ "rom"     ,     rom },
	{ "search"  ,  search },
	
	// DS18B12 Function commands:
	{ "convert" , convert },
	{ "pad"     ,     pad },
	{ "power"   ,   power },
	{ "restore" , restore },
	{ "save"    ,    save },
	{ "write"   ,   write },
    } ;
    argL->pop(map)(rpi,argL) ;
}
