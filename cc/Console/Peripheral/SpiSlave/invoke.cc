// BSD 2-Clause License, see github.com/ma16/rpio

#include "../invoke.h"
#include <Rpi/SpiSlave.h>
#include <Neat/U32.h>
#include <Ui/strto.h>
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
    
    Rpi::SpiSlave spi(rpi) ;
    auto ctrl = spi.getControl() ;
    
    while (!argL->empty())
    {
	auto arg = argL->pop() ;
	if (false) ;
	else if (arg ==         "all") set<                     ~0u>(&ctrl,argL) ;
	
	else if (arg ==      "enable") set<Rpi::SpiSlave::   Enable>(&ctrl,argL) ;
	else if (arg ==         "spi") set<Rpi::SpiSlave::      Spi>(&ctrl,argL) ;
	else if (arg ==         "i2c") set<Rpi::SpiSlave::      I2c>(&ctrl,argL) ;
	else if (arg ==        "cpha") set<Rpi::SpiSlave::     Cpha>(&ctrl,argL) ;
	else if (arg ==        "cpol") set<Rpi::SpiSlave::     Cpol>(&ctrl,argL) ;
	else if (arg ==     "en-stat") set<Rpi::SpiSlave::   EnStat>(&ctrl,argL) ;
	else if (arg ==     "en-ctrl") set<Rpi::SpiSlave::   EnCtrl>(&ctrl,argL) ;
	else if (arg ==         "brk") set<Rpi::SpiSlave::      Brk>(&ctrl,argL) ;
	else if (arg ==       "en-tx") set<Rpi::SpiSlave::     EnTx>(&ctrl,argL) ;
	else if (arg ==       "en-rx") set<Rpi::SpiSlave::     EnRx>(&ctrl,argL) ;
	else if (arg == "inv-rx-full") set<Rpi::SpiSlave::InvRxFull>(&ctrl,argL) ;
	else if (arg ==     "en-test") set<Rpi::SpiSlave::   EnTest>(&ctrl,argL) ;
	else if (arg ==     "en-host") set<Rpi::SpiSlave::   EnHost>(&ctrl,argL) ;
	else if (arg == "inv-tx-full") set<Rpi::SpiSlave::InvTxFull>(&ctrl,argL) ;
	
	else throw std::runtime_error("not supported option:<"+arg+'>') ;
    }
    spi.setControl(ctrl) ;
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
    Rpi::SpiSlave spi(rpi) ;
    
    auto flags = spi.flags() ;
    auto ctrl = spi.getControl() ;
    auto status = spi.status() ;

    std::cout << "Status Register (0x" << hex(status) << ")\n"
	      << "rx-over .... " << get<Rpi::SpiSlave:: RxOver>(status) << '\n'
	      << "tx-under ... " << get<Rpi::SpiSlave::TxUnder>(status) << "\n\n" ; 
    
    std::cout << "Control Register (0x" << hex(ctrl) << ")\n"
	      << "enable ........ " << get<Rpi::SpiSlave::   Enable>(ctrl) << '\n'
	      << "spi ........... " << get<Rpi::SpiSlave::      Spi>(ctrl) << '\n'
	      << "i2c ........... " << get<Rpi::SpiSlave::      I2c>(ctrl) << '\n'
	      << "cpha .......... " << get<Rpi::SpiSlave::     Cpha>(ctrl) << '\n'
	      << "cpol .......... " << get<Rpi::SpiSlave::     Cpol>(ctrl) << '\n'
	      << "en-stat ....... " << get<Rpi::SpiSlave::   EnStat>(ctrl) << '\n'
	      << "en-ctrl ....... " << get<Rpi::SpiSlave::   EnCtrl>(ctrl) << '\n'
	      << "brk ........... " << get<Rpi::SpiSlave::      Brk>(ctrl) << '\n'
	      << "en-tx ......... " << get<Rpi::SpiSlave::     EnTx>(ctrl) << '\n'
	      << "en-rx ......... " << get<Rpi::SpiSlave::     EnRx>(ctrl) << '\n'
	      << "inv-rx-full ... " << get<Rpi::SpiSlave::InvRxFull>(ctrl) << '\n'
	      << "en-test ....... " << get<Rpi::SpiSlave::   EnTest>(ctrl) << '\n'
	      << "en-host ....... " << get<Rpi::SpiSlave::   EnHost>(ctrl) << '\n'
	      << "inv-tx-full ... " << get<Rpi::SpiSlave::InvTxFull>(ctrl) << "\n\n" ;
    
    std::cout << "Flag Register (0x" << hex(flags) << ")\n"
	      << "tx-busy .... " << get<Rpi::SpiSlave:: TxBusy>(flags) << '\n' 
	      << "rx-empty ... " << get<Rpi::SpiSlave::RxEmpty>(flags) << '\n' 
	      << "tx-full .... " << get<Rpi::SpiSlave:: TxFull>(flags) << '\n' 
	      << "rx-full .... " << get<Rpi::SpiSlave:: RxFull>(flags) << '\n' 
	      << "tx-empty ... " << get<Rpi::SpiSlave::TxEmpty>(flags) << '\n' 
	      << "rx-busy .... " << get<Rpi::SpiSlave:: RxBusy>(flags) << '\n' 
	      << "tx-level ... " << get<Rpi::SpiSlave::TxLevel>(flags) << '\n' 
	      << "rx-level ... " << get<Rpi::SpiSlave::RxLevel>(flags) << "\n\n" ; 
}

static void read(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    std::cout << std::hex << spi.read() << std::endl ;
}

static void readAll(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    std::deque<uint32_t> rx ;
    while (0 == (spi.flags() & Rpi::SpiSlave::RxEmpty))
	rx.push_back(spi.read()) ;
    std::cout << rx.size() << ":" ;
    for (auto w: rx)
	std::cout << ' ' << std::hex << (w & 0xff) ;
    std::cout << std::endl ;
}

static void readTx(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    std::cout << std::hex << spi.readTx() << std::endl ;
}

static void reset(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    spi.reset() ;
}

static void clearTx(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (!argL->empty() && argL->peek() == "help")
    {
	std::cout << "no arguments" << std::endl ;
	return ;
    }
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ; 
    auto ctrl = spi.getControl() ; // backup
    uint32_t level ;
    do
    {
	level = spi.flags() & Rpi::SpiSlave::TxLevel ;
	spi.setControl(0) ;
	spi.setControl(Rpi::SpiSlave::Enable | Rpi::SpiSlave::EnTx) ;
    }
    while (level != 0) ;
    spi.setControl(ctrl) ; // recover
}

static void write(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "argument: the-byte-to-send\n"
		  << std::flush ;
	return ;
    }
    auto w = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    spi.write(w) ;
}

static void writeRx(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    if (argL->empty() || argL->peek() == "help")
    {
	std::cout << "argument: the-byte-to-send\n"
		  << std::flush ;
	return ;
    }
    auto w = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    Rpi::SpiSlave spi(rpi) ;
    spi.writeRx(w) ;
}

void Console::Peripheral::SpiSlave::
invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
    // note: pin mode (ALT3) has to be enabled by user
    // i.e. BCM pins MISO=18, SCLK=19, MOSI=20, CE=21
  
    if (argL->empty() || argL->peek() == "help")
    { 
	std::cout << "arguments: MODE [help]\n"
		  << '\n'
		  << "MODE | clear-tx  # clear tx-fifo\n"
		  << "     | control   # set control register\n"
		  << "     | read      # read octet from rx-fifo\n"
		  << "     | read-all  # read all octets from rx-fifo\n"
		  << "     | read-tx   # read octet from tx-fifo (test-mode)\n"
		  << "     | reset     # reset status register\n"
		  << "     | status    # display registers\n"
		  << "     | write     # write word to tx-fifo\n"
		  << "     | write-rx  # write word to rx-fifo (test-mode)\n"
		  << std::flush ;
	return ;
    }
    std::string arg = argL->pop() ;
    if (false) ;

    else if (arg == "clear-tx") clearTx(rpi,argL) ; 
    else if (arg ==  "control") control(rpi,argL) ; 
    else if (arg ==     "read")    read(rpi,argL) ; 
    else if (arg == "read-all") readAll(rpi,argL) ; 
    else if (arg ==  "read-tx")  readTx(rpi,argL) ; 
    else if (arg ==    "reset")   reset(rpi,argL) ; 
    else if (arg ==   "status")  status(rpi,argL) ;
    else if (arg ==    "write")   write(rpi,argL) ; 
    else if (arg == "write-rx") writeRx(rpi,argL) ; 
    
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
