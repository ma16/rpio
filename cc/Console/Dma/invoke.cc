// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// DMA setup and test
// --------------------------------------------------------------------

#include "../rpio.h"
#include <Neat/cast.h>
#include <Neat/safe_int.h>
#include <Linux/Shm.h>
#include <Posix/base.h> // nanosleep
#include <Rpi/Dma.h>
#include <Rpi/GpuMem.h>
#include <Rpi/Timer.h>
#include <Rpi/Ui/Dma.h>
#include <Ui/strto.h>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace Console::Dma ;

// --------------------------------------------------------------------
    
static std::string mkhdr(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i/10 << ' ' ;
  os << '\n' ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << i%10 << ' ' ;
  return os.str() ;
}

static std::string mksep(uint32_t mask)
{
  std::ostringstream os ;
  for (auto i=0 ; i<32 ; ++i) 
    if (mask & (1u<<i))
      os << "--" ;
  return os.str() ;
}

static std::string mkstr(uint32_t mask,uint32_t bits)
{
  char s[32*2] ;
  auto ofs = 0u ;
  for (uint32_t m=1u ; m!=0 ; m<<=1) {
    if (m & mask) {
      s[ofs++] = (bits & m) ? '1' : '0' ;
      s[ofs++] = ' ' ;
    }
  }  
  return std::string(s,ofs) ;
} 
// [todo] deduplicate code (see Gpio)

// --------------------------------------------------------------------
    
static void enableInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto index = Ui::strto(argL->pop(),Neat::Enum<unsigned,14>()) ;
  // ...index for an uint15_5 bit-mask
  // ...[future] introduce a generic bit/mask option (as for GPIO pins)
  auto off = argL->pop_if("off") ;
  argL->finalize() ;
  Rpi::Dma::Ctrl dma(rpi) ;
  auto mask = 1u << index.value() ;
  auto bits = dma.enabled().value() ;
  if (off) bits &= ~mask ;
  else     bits |=  mask ;
  dma.enable(Rpi::Dma::Ctrl::uint15_t::coset(bits)) ;
}

// --------------------------------------------------------------------
    
static void resetInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto index = Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index()) ;
  argL->finalize() ;
  Rpi::Dma::Ctrl(rpi).channel(index).reset() ;
}

// --------------------------------------------------------------------
    
static void debugStatus(Rpi::Peripheral *rpi)
{
  std::cout << "# Lite Ver State Id Writes Read Fifo Last\n"
	    << "-----------------------------------------\n" ;
  std::cout << std::hex ;
  Rpi::Dma::Ctrl::Index i ;
  do {
    auto d = Rpi::Dma::Ctrl(rpi).channel(i).getDebug() ;
    
    std::cout << std::setw(1) << i.value()
	      << std::setw(5) << d.   lite().get().value()
	      << std::setw(4) << d.version().get().value()
	      << std::setw(6) << d.  state().get().value()
	      << std::setw(3) << d.     id().get().value()
	      << std::setw(7) << d. writes().get().value()
      	      << std::setw(5) << d. rerror().get().value()
	      << std::setw(5) << d. ferror().get().value()
	      << std::setw(5) << d. lerror().get().value()
	      << '\n' ;
  }
  while (i.next()) ;
  std::cout << std::flush ;
}

static void fullStatus(Rpi::Peripheral *rpi)
{
  std::cout << "#       CS   BLK_AD       TI   SRC_AD  DEST_AD TXFR_LEN   STRIDE   NEXTBK    DEBUG\n"
	    << "----------------------------------------------------------------------------------\n" ;
  std::cout << std::hex ;
  Rpi::Dma::Ctrl dma(rpi) ;
  Rpi::Dma::Ctrl::Index i ;
  do {
    std::cout << i.value() << ' ' << dma.channel(i).toStr() << '\n' ;
  }
  while (i.next()) ;
  std::cout << std::flush ;
}

static void briefStatus(Rpi::Peripheral *rpi)
{
  Rpi::Dma::Ctrl dma(rpi) ;
  static auto const mask = 0xffffu ;
  std::cout << mkhdr(mask) << '\n'
	    << mksep(mask) << '\n' 
	    << mkstr(mask,dma.enabled().value()) << "enabled\n" 
	    << mkstr(mask,dma.status())          << "interrupt\n" ;
  Rpi::Dma::Ctrl::Index i ;
  do {
    std::cout << (0 != (dma.channel(i).getCs().active().bits())) << ' ' ;
  }
  while (i.next()) ;
  std::cout << "busy" << std::endl ;
}

static void statusInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto opt = argL->pop_if({"-d","-v"}) ;
  argL->finalize() ;
  if (opt) {
    if (*opt == 0) debugStatus(rpi) ;
    else            fullStatus(rpi) ;
  }
  else briefStatus(rpi) ;
}

// --------------------------------------------------------------------
    
static void startInvoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  auto channel = Rpi::Dma::Ctrl(rpi).channel(Ui::strto(argL->pop(),Rpi::Dma::Ctrl::Index())) ;
  auto cb = Ui::strto<uint32_t>(argL->pop()) ;
  auto cs = Rpi::Ui::Dma::getCs(argL,Rpi::Dma::Cs()) ;
  argL->finalize() ;
  channel.setup(Rpi::Bus::Address(cb),cs) ;
  channel.start() ;
  while (0 != (channel.getCs().active().bits()))
    ; // busy loop to get coherency defect
      //Posix::nanosleep(1E+3) ;
}

// --------------------------------------------------------------------
    
static void tiInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto ti = Rpi::Ui::Dma::getTi(argL,Rpi::Dma::Ti::Word()) ;
  argL->finalize() ;
  std::cout << std::hex << ti.value() << std::endl ;
}

// --------------------------------------------------------------------
    
void Console::Dma::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE\n"
	      << '\n'
	      << "MODE : enable INDEX [off]  # enable/disable DMA channel\n"
	      << "     | start INDEX CB CS   # start DMA transfer\n"
	      << "     | reset INDEX         # end transmission\n"
	      << "     | status [-d|-v]      # show status of all channels\n"
	      << "     | ti TI+              # create TI word from flags\n"
	      << '\n'
	      << "DMA channels:\n"
	      << "INDEX : 0..15              # one of the 16 DMA channels\n"
	      << '\n'
	      << "Control and Status Register CS:\n"
	      << Rpi::Ui::Dma::csSynopsis()
	      << '\n'
	      << "Transfer Information TI:\n"
	      << Rpi::Ui::Dma::tiSynopsis()
	      << std::flush ;
    return ;
    // [future] INDEX 0..6 vs 7..14 (Lite) vs 15 (not operational at all)
    // [future] support the reset of interrupt status flag
  }
  std::string arg = argL->pop() ;
  if      (arg == "enable") enableInvoke(rpi,argL) ;
  else if (arg ==  "reset")  resetInvoke(rpi,argL) ;
  else if (arg ==  "start")  startInvoke(rpi,argL) ;
  else if (arg == "status") statusInvoke(rpi,argL) ;
  else if (arg ==     "ti")     tiInvoke(rpi,argL) ;
  
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
