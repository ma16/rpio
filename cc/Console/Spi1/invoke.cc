// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include <Neat/uint.h>
#include <Neat/U32.h>
#include <Rpi/Spi1.h>
#include <Ui/strto.h>
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>

template<uint32_t Mask> static void set(uint32_t *p,Ui::ArgL *argL)
{
    static_assert(Neat::U32::Contiguous<Mask>(),"bit-mask not contiguous") ;
    assert(p != nullptr) ;
    
    auto ofs = Neat::U32::AlignRight<Mask>() ;
    auto max = Mask >> ofs ;
    
    auto i = Ui::strto<uint32_t>(argL->pop()) ;
    if (i > max)
    {
	std::ostringstream os ;
	os << "domain error: " << i << " exceeds " << max ;
	throw std::runtime_error(os.str()) ;
    }
    (*p) = ((*p) & ~Mask) | (i << ofs) ;
}

static void control(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: (ATTR VALUE)*\n"
		  << "refer to the output of the status command for valid attributes\n"
		  << std::flush ;
	return ;
    }
    
    Rpi::Spi1 spi(rpi) ;
    auto c0 = spi.getControl0() ;
    auto c1 = spi.getControl1() ;
    
    while (!argL->empty())
    {
	auto arg = argL->pop() ;

	if (false) ;
	
	else if (arg == "cs#0"    ) set<Rpi::Spi1::Cs0    >(&c0,argL) ;
	else if (arg == "cs#1"    ) set<Rpi::Spi1::Cs1    >(&c0,argL) ;
	else if (arg == "cs#2"    ) set<Rpi::Spi1::Cs2    >(&c0,argL) ;
	else if (arg == "cs-hold" ) set<Rpi::Spi1::CsHold >(&c1,argL) ;
	else if (arg == "enable"  ) set<Rpi::Spi1::Enable >(&c0,argL) ;
	else if (arg == "flush"   ) set<Rpi::Spi1::Flush  >(&c0,argL) ;
	else if (arg == "idle-clk") set<Rpi::Spi1::IdleClk>(&c0,argL) ;
	else if (arg == "irq-idle") set<Rpi::Spi1::IrqIdle>(&c1,argL) ;
	else if (arg == "irq-fifo") set<Rpi::Spi1::IrqFifo>(&c1,argL) ;
	else if (arg == "len"     ) set<Rpi::Spi1::Len    >(&c0,argL) ;
	else if (arg == "rx-keep" ) set<Rpi::Spi1::RxKeep >(&c1,argL) ;
	else if (arg == "rx-lh"   ) set<Rpi::Spi1::RxLh   >(&c0,argL) ;
	else if (arg == "rx-msb"  ) set<Rpi::Spi1::RxMsb  >(&c1,argL) ;
	else if (arg == "rx-post" ) set<Rpi::Spi1::RxPost >(&c0,argL) ;
	else if (arg == "speed"   ) set<Rpi::Spi1::Speed  >(&c0,argL) ;
	else if (arg == "tx-cx"   ) set<Rpi::Spi1::TxCs   >(&c0,argL) ;
	else if (arg == "tx-lh"   ) set<Rpi::Spi1::TxLh   >(&c0,argL) ;
	else if (arg == "tx-hold" ) set<Rpi::Spi1::TxHold >(&c0,argL) ;
	else if (arg == "tx-len"  ) set<Rpi::Spi1::TxLen  >(&c0,argL) ;
	else if (arg == "tx-msb"  ) set<Rpi::Spi1::TxMsb  >(&c0,argL) ;

	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    spi.setControl0(c0) ;
    spi.setControl1(c1) ;
}

static std::string hex(uint32_t i)
{
    std::ostringstream os ;
    os << std::hex << i ;
    return os.str() ;
}

template<uint32_t Mask> static uint32_t get(uint32_t bucket)
{
    return (bucket & Mask) >> Neat::U32::AlignRight<Mask>() ;
}

static void status(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::Spi1 spi(rpi) ;
    auto s = spi.status() ;
    auto c0 = spi.getControl0() ;
    auto c1 = spi.getControl1() ;

    std::cout << "Status Register (0x" << hex(s) << ")\n"
	      << "left ....... " << get<Rpi::Spi1::   Left>(s) << '\n' 
	      << "busy ....... " << get<Rpi::Spi1::   Busy>(s) << '\n' 
	      << "rx-empty ... " << get<Rpi::Spi1::RxEmpty>(s) << '\n' 
	      << "rx-full .... " << get<Rpi::Spi1:: RxFull>(s) << '\n' 
	      << "tx-empty ... " << get<Rpi::Spi1::TxEmpty>(s) << '\n' 
	      << "tx-full .... " << get<Rpi::Spi1:: TxFull>(s) << '\n' 
	      << "rx-size .... " << get<Rpi::Spi1:: RxSize>(s) << '\n' 
	      << "tx-size  ... " << get<Rpi::Spi1:: TxSize>(s) << "\n\n"  ;

    std::cout << "Control Register #0 (0x" << hex(c0) << ")\n"
	      << "len ........ " << get<Rpi::Spi1::    Len>(c0) << '\n' 
	      << "tx-msb ..... " << get<Rpi::Spi1::  TxMsb>(c0) << '\n' 
	      << "idle-clk ... " << get<Rpi::Spi1::IdleClk>(c0) << '\n' 
	      << "tx-lh ...... " << get<Rpi::Spi1::   TxLh>(c0) << '\n' 
	      << "flush ...... " << get<Rpi::Spi1::  Flush>(c0) << '\n' 
	      << "rx-lh ...... " << get<Rpi::Spi1::   RxLh>(c0) << '\n' 
	      << "enable ..... " << get<Rpi::Spi1:: Enable>(c0) << '\n' 
	      << "tx-hold .... " << get<Rpi::Spi1:: TxHold>(c0) << '\n' 
	      << "tx-len ..... " << get<Rpi::Spi1::  TxLen>(c0) << '\n' 
	      << "tx-cs ...... " << get<Rpi::Spi1::   TxCs>(c0) << '\n' 
	      << "rx-post .... " << get<Rpi::Spi1:: RxPost>(c0) << '\n' 
	      << "cs#0 ....... " << get<Rpi::Spi1::    Cs0>(c0) << '\n' 
	      << "cs#1 ....... " << get<Rpi::Spi1::    Cs1>(c0) << '\n' 
	      << "cs#2 ....... " << get<Rpi::Spi1::    Cs2>(c0) << '\n' 
	      << "speed ...... " << get<Rpi::Spi1::  Speed>(c0) << "\n\n" ;

    std::cout << "Control Register #1 (0x" << hex(c1) << ")\n"
	      << "rx-keep .... " << get<Rpi::Spi1:: RxKeep>(c1) << '\n' 
	      << "rx-msb ..... " << get<Rpi::Spi1::  RxMsb>(c1) << '\n' 
	      << "irq-idle ... " << get<Rpi::Spi1::IrqIdle>(c1) << '\n' 
	      << "irq-fifo ... " << get<Rpi::Spi1::IrqFifo>(c1) << '\n' 
	      << "cs-hold .... " << get<Rpi::Spi1:: CsHold>(c1) << '\n' ;
}

static void write(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "argument: [-c] WORD\n"
		  << "\n"
		  << "  -c: don't de-assert CS\n"
		  << "WORD: the word to send\n"
		  << std::flush ;
	return ;
    }
    auto continuous = argL->pop_if("-c") ;
    auto w = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Spi1 spi(rpi) ;
    if (continuous) spi.writeNext(w) ;
    else            spi.writeEnd (w) ;
}

static void peek(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::Spi1 spi(rpi) ;
    std::cout << std::hex << spi.peek() << std::endl ;
}

static void read(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::Spi1 spi(rpi) ;
    std::cout << std::hex << spi.read() << std::endl ;
}

static void xfer(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "argument: [-c] WORD*\n"
		  << "\n"
		  << "  -c: don't de-assert CS between words\n"
		  << "WORD: the word to send\n"
		  << std::flush ;
	return ;
    }
    auto continuous = argL->pop_if("-c") ;
    std::deque<uint32_t> tx ;
    while (!argL->empty())
	tx.push_back(Ui::strto<uint32_t>(argL->pop())) ;
    argL->finalize() ;
    Rpi::Spi1 spi(rpi) ;
    std::vector<uint32_t> rx ; rx.reserve(tx.size()) ;
    spi.xfer(tx,&rx,continuous) ;
    for (auto w: rx)
    {
	std::cout << std::hex << w << ' ' ;
    }
    std::cout << std::endl ;
}

static void rate(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "arguments: N [-c] [-w WORD]\n"
		  << "\n"
		  << "   N: number of words to transceive\n"
		  << "  -c: don't de-assert CS between words\n"
		  << "WORD: the word to send; else 0xffffff\n"
		  << std::flush ;
	return ;
    }
    auto n = Ui::strto<size_t>(argL->pop()) ;
    auto continuous = argL->pop_if("-c") ;
    auto w = Ui::strto<uint32_t>(argL->option("-w","0x00ffffff")) ;
    argL->finalize() ;
    std::deque<uint32_t> tx(n,w) ;
    std::vector<uint32_t> rx ; rx.reserve(tx.size()) ;
    Rpi::Spi1 spi(rpi) ;
    auto t0 = std::chrono::steady_clock::now() ;
    spi.xfer(tx,&rx,continuous) ;
    auto t1 = std::chrono::steady_clock::now() ;
    std::cout.setf(std::ios::scientific) ;
    std::cout.precision(2) ;
    std::cout << n/std::chrono::duration<double>(t1-t0).count() << "/s" << std::endl ;
}

void Console::Spi1::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE | control  # set control registers\n"
		  << "     | peek     # peek word from rx-fifo\n"
		  << "     | rate     # throughput test\n"
		  << "     | read     # read word from rx-fifo\n"
		  << "     | status   # display control and status registers\n"
		  << "     | write    # write word to tx-fifo\n"
		  << "     | xfer     # transceive multiple words\n"
		  << std::flush ;
	return ;
    }
    std::string arg = argL->pop() ;
    if (false) ;
    
    else if (arg == "control") control(rpi,argL) ; 
    else if (arg ==    "peek")    peek(rpi,argL) ; 
    else if (arg ==    "rate")    rate(rpi,argL) ; 
    else if (arg ==    "read")    read(rpi,argL) ; 
    else if (arg ==  "status")  status(rpi,argL) ; 
    else if (arg ==   "write")   write(rpi,argL) ; 
    else if (arg ==    "xfer")    xfer(rpi,argL) ;
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
