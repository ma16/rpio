// BSD 2-Clause License, see github.com/ma16/rpio

#include "../rpio.h"
#include "Dma.h"
#include <Neat/uint.h>
#include <Neat/U32.h>
#include <Rpi/Spi0.h>
#include <Rpi/Timer.h>
#include <Ui/strto.h>
#include <arm/arm.h>
#include <iomanip>
#include <iostream>
#include <vector>

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
    
    Rpi::Spi0 spi(rpi) ;
    auto c = spi.getControl() ;
    while (!argL->empty())
    {
	auto arg = argL->pop() ;
	if (false) ;
	
	else if (arg ==       "cs") set<Rpi::Spi0::     Cs>(&c,argL) ;
	else if (arg ==     "cpha") set<Rpi::Spi0::   Cpha>(&c,argL) ;
	else if (arg ==     "cpol") set<Rpi::Spi0::   Cpol>(&c,argL) ;
	else if (arg == "clear-tx") set<Rpi::Spi0::ClearTx>(&c,argL) ;
	else if (arg == "clear-rx") set<Rpi::Spi0::ClearRx>(&c,argL) ;
	else if (arg ==    "cspol") set<Rpi::Spi0::  Cspol>(&c,argL) ;
	else if (arg ==       "ta") set<Rpi::Spi0::     Ta>(&c,argL) ;
	else if (arg ==    "dmaen") set<Rpi::Spi0::  Dmaen>(&c,argL) ;
	else if (arg ==     "intd") set<Rpi::Spi0::   Intd>(&c,argL) ;
	else if (arg ==     "intr") set<Rpi::Spi0::   Intr>(&c,argL) ;
	else if (arg ==     "adcs") set<Rpi::Spi0::   Adcs>(&c,argL) ;
	else if (arg ==      "ren") set<Rpi::Spi0::    Ren>(&c,argL) ;
	else if (arg ==      "len") set<Rpi::Spi0::    Len>(&c,argL) ;
	else if (arg ==   "cspol0") set<Rpi::Spi0:: Cspol0>(&c,argL) ;
	else if (arg ==   "cspol1") set<Rpi::Spi0:: Cspol1>(&c,argL) ;
	else if (arg ==   "cspol2") set<Rpi::Spi0:: Cspol2>(&c,argL) ;
	else if (arg ==   "dmaLen") set<Rpi::Spi0:: DmaLen>(&c,argL) ;
	else if (arg ==  "lenLong") set<Rpi::Spi0::LenLong>(&c,argL) ;
	
	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    spi.setControl(c) ;
}

static void div(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto div = Ui::strto<uint16_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Spi0 spi(rpi) ;
    spi.setDivider(div) ;
}

static void dlen(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    auto dlen = Ui::strto<uint16_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::Spi0 spi(rpi) ;
    spi.setDlen(dlen) ;
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
    argL->finalize() ;
    Rpi::Spi0 spi(rpi) ;
    auto c = spi.getControl() ;
    auto d = spi.getDivider() ;
    auto l = spi.getDlen() ;
    auto m = spi.getDc() ;
    
    std::cout << "Control Register (0x" << hex(c) << ")\n"
	      << "cs ......... " << get<Rpi::Spi0::     Cs>(c) << '\n'
	      << "cpha ....... " << get<Rpi::Spi0::   Cpha>(c) << '\n'
	      << "cpol ....... " << get<Rpi::Spi0::   Cpol>(c) << '\n'
	      << "cspol ...... " << get<Rpi::Spi0::  Cspol>(c) << '\n'
	      << "ta ......... " << get<Rpi::Spi0::     Ta>(c) << '\n'
	      << "dmaen ...... " << get<Rpi::Spi0::  Dmaen>(c) << '\n'
	      << "intd ....... " << get<Rpi::Spi0::   Intd>(c) << '\n'
	      << "intr ....... " << get<Rpi::Spi0::   Intr>(c) << '\n'
	      << "adcs ....... " << get<Rpi::Spi0::   Adcs>(c) << '\n'
	      << "ren ........ " << get<Rpi::Spi0::    Ren>(c) << '\n'
	      << "len ........ " << get<Rpi::Spi0::    Len>(c) << '\n'
	      << "done ....... " << get<Rpi::Spi0::   Done>(c) << '\n'
	      << "rxd ........ " << get<Rpi::Spi0::    Rxd>(c) << '\n'
	      << "txd ........ " << get<Rpi::Spi0::    Txd>(c) << '\n'
	      << "rxr ........ " << get<Rpi::Spi0::    Rxr>(c) << '\n'
	      << "rxf ........ " << get<Rpi::Spi0::    Rxf>(c) << '\n'
	      << "cspol0 ..... " << get<Rpi::Spi0:: Cspol0>(c) << '\n'
	      << "cspol1 ..... " << get<Rpi::Spi0:: Cspol1>(c) << '\n'
	      << "cspol2 ..... " << get<Rpi::Spi0:: Cspol2>(c) << '\n'
	      << "dma-len .... " << get<Rpi::Spi0:: DmaLen>(c) << '\n'
	      << "len-long ... " << get<Rpi::Spi0::LenLong>(c) << "\n\n" ;
      
    std::cout << "Divider Register: 0x" << hex(d) << "\n\n" ;
    
    std::cout << "Data-Length Register: 0x" << hex(l) << "\n\n" ;

    std::cout << "DMA control Register (0x" << hex(m) << ")\n"
	      << "tx-req ..... " << get<Rpi::Spi0::  TxReq>(m) << '\n'
	      << "tx-panic ... " << get<Rpi::Spi0::TxPanic>(m) << '\n'
	      << "rx-req ..... " << get<Rpi::Spi0::  RxReq>(m) << '\n'
	      << "rx-panic ... " << get<Rpi::Spi0::RxPanic>(m) << "\n\n" ;
    
}

static void run(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    argL->finalize() ;
    std::vector<uint8_t> tx ;
    while (true)
    {
	std::string byte ;
	std::cin >> byte ;
	if (!std::cin)
	    break ;
	tx.push_back(Ui::strto<uint8_t>(byte)) ;
    }
    std::vector<uint8_t> rx(tx.size()) ;
    Rpi::Spi0 spi(rpi) ;
    spi.transceive(tx.size(),&tx[0],&rx[0]) ;
    for (decltype(rx)::size_type i=0 ; i<rx.size() ; ++i)
	std::cout << "0x" << std::hex << int(rx[i]) << ' ' ;
    std::cout << '\n' ;
}

void Console::Spi0::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help") { 
	std::cout << "arguments: MODE\n"
		  << '\n'
		  << "MODE | control [help] # set control status word\n"
		  << "     | dma     [help] # invoke DMA based transfer\n"
		  << "     | div UINT16     # set clock pulse divider\n"
		  << "     | dlen LEN       # set DLEN register\n"
		  << "     | run            # read bytes (ASCII) from stdin and send them\n"
		  << "     | status         # display status\n"
		  << std::flush ;
	return ;
    }
    std::string arg = argL->pop() ;
    if (false) ;
    
    else if (arg == "control") control(rpi,argL) ; 
    else if (arg ==     "div")     div(rpi,argL) ; 
    else if (arg ==    "dlen")    dlen(rpi,argL) ;
    else if (arg ==     "run")     run(rpi,argL) ; 
    else if (arg ==  "status")  status(rpi,argL) ; 
    
    else if (arg ==     "dma")  Dma::invoke(rpi,argL) ;
    // ...should be split off
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
