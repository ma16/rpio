// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include <Ui/strto.h>
#include <iomanip> // setw

// --------------------------------------------------------------------

Rpi::Dma::Cs Console::Dma::Lib::optCs(Ui::ArgL *argL,Rpi::Dma::Cs cs)
{
  while (!argL->empty()) {
    auto arg = argL->peek() ;
    
    if      (arg == "--cs-disdebug"                   ) cs.disdebug() = false ;
    else if (arg == "--cs+disdebug"                   ) cs.disdebug() =  true ;
    
    else if (arg == "--cs-wait-for-outstanding-writes") cs.    wait() = false ;
    else if (arg == "--cs+wait-for-outstanding-writes") cs.    wait() =  true ;
    
    else if (arg == "--cs=panic-priority"             ) cs.   panic() = Ui::strto(argL->pop(),Rpi::Dma::Cs::Panic::Uint()) ;
    else if (arg == "--cs=priority"                   ) cs.priority() = Ui::strto(argL->pop(),Rpi::Dma::Cs::Panic::Uint()) ;
    
    else break ;
    argL->pop() ;
  }
  return cs ;
}

// --------------------------------------------------------------------

Rpi::Dma::Ti Console::Dma::Lib::optTi(Ui::ArgL *argL,Rpi::Dma::Ti ti)
{
  while (!argL->empty()) {
    auto arg = argL->peek() ;
    
    if      (arg == "--ti-no-wide-bursts") ti.noWideBursts() = false ;
    else if (arg == "--ti+no-wide-bursts") ti.noWideBursts() =  true ;
    
    else if (arg == "--ti-src-ignore"    ) ti.   srcIgnore() = false ;
    else if (arg == "--ti+src-ignore"    ) ti.   srcIgnore() =  true ;
    
    else if (arg == "--ti-src-dreq"      ) ti.     srcDreq() = false ;
    else if (arg == "--ti+src-dreq"      ) ti.     srcDreq() =  true ;

    else if (arg == "--ti-src-width"     ) ti.    srcWidth() = false ;
    else if (arg == "--ti+src-width"     ) ti.    srcWidth() =  true ;

    else if (arg == "--ti-src-inc"       ) ti.      srcInc() = false ;
    else if (arg == "--ti+src-inc"       ) ti.      srcInc() =  true ;

    else if (arg == "--ti-dest-ignore"   ) ti.  destIgnore() = false ;
    else if (arg == "--ti+dest-ignore"   ) ti.  destIgnore() =  true ;

    else if (arg == "--ti-dest-dreq"     ) ti.    destDreq() = false ;
    else if (arg == "--ti+dest-dreq"     ) ti.    destDreq() =  true ;

    else if (arg == "--ti-dest-width"    ) ti.   destWidth() = false ;
    else if (arg == "--ti+dest-width"    ) ti.   destWidth() =  true ;

    else if (arg == "--ti-dest-inc"      ) ti.     destInc() = false ;
    else if (arg == "--ti+dest-inc"      ) ti.     destInc() =  true ;

    else if (arg == "--ti-wait-resp"     ) ti.    waitResp() = false ;
    else if (arg == "--ti+wait-resp"     ) ti.    waitResp() =  true ;

    else if (arg == "--ti-tdmode"        ) ti.      tdmode() = false ;
    else if (arg == "--ti+tdmode"        ) ti.      tdmode() =  true ;

    else if (arg == "--ti-inten"         ) ti.       inten() = false ;
    else if (arg == "--ti+inten"         ) ti.       inten() =  true ;
    
    else if (arg == "--ti=waits"         ) ti.       waits() = Ui::strto(argL->pop(),Rpi::Dma::Ti::      Waits::Uint()) ;
    else if (arg == "--ti=permap"        ) ti.      permap() = Ui::strto(argL->pop(),Rpi::Dma::Ti::     Permap::Uint()) ;
    else if (arg == "--ti=burst-length"  ) ti. burstLength() = Ui::strto(argL->pop(),Rpi::Dma::Ti::BurstLength::Uint()) ;

    else break ;
    argL->pop() ;
  }
  return ti ;
}

// --------------------------------------------------------------------

void Console::Dma::Lib::Control::write(Rpi::Dma::Ti ti,Rpi::Bus::Address src,Rpi::Bus::Address dst,uint32_t nbytes,uint32_t stride)
{
  // [todo] this function should not access DMA control blocks directly
  //   but use some Rpi::Dma::... methods for that
  
  assert(i < mem_->nbytes() / 32) ;
  if (i > 0) {
    // let previous block link here
    auto p = mem_->as<uint32_t volatile(*)[8]>() + (i-1) ;
    auto self = mem_->phys(32*i).first ;
    (*p)[5] = self.value() ;
  }
  auto p = mem_->as<uint32_t volatile(*)[8]>() + i ;
  (*p)[0] = ti.value() ;
  (*p)[1] = src.value() ;
  (*p)[2] = dst.value() ;
  (*p)[3] = nbytes ;
  (*p)[4] = stride ;
  (*p)[5] = 0 ;
  (*p)[6] = 0 ; // Reserved - set to zero.
  (*p)[7] = 0 ; // Reserved - set to zero.
  ++i ;
}

void Console::Dma::Lib::Control::repeat(size_t ofs)
{
  assert(ofs < i) ;
  auto p = mem_->as<uint32_t volatile(*)[8]>() + (i-1) ;
  auto to = mem_->phys(32*ofs).first ;
  (*p)[5] = to.value() ;
}

std::string Console::Dma::Lib::Control::toStr(Rpi::Bus::Memory *cb)
{
  std::ostringstream os ;
  os << "    ADDR       TI      SRC      DST     SIZE   STRIDE     NEXT\n" ;
  os << std::hex ;
  size_t i = 0 ;
  while (true) {
    if ((i+1) * 32 > cb->nbytes())
      break ;
    auto p = cb->as<uint32_t volatile(*)[8]>() + i ;
    os << std::setw(8) << cb->phys(32*i).first.value()
       << std::setw(9) << (*p)[0]
       << std::setw(9) << (*p)[1]
       << std::setw(9) << (*p)[2]
       << std::setw(9) << (*p)[3]
       << std::setw(9) << (*p)[4]
       << std::setw(9) << (*p)[5] << '\n' ;
    if (p[5] == 0)
      break ;
    ++i ;
  }
  return os.str() ;
}

void Console::Dma::Lib::write(Control *ctl,Rpi::Dma::Ti ti,Rpi::Bus::Address src,Rpi::Bus::Memory *dst,uint32_t dofs,uint32_t nbytes)
{
  auto section = dst->phys(dofs) ;
  if (section.second < nbytes)
    throw std::runtime_error("PhysMem:access out of range") ;
  ctl->write(ti,src,section.first,nbytes,0) ;
}

void Console::Dma::Lib::write(Control *ctl,Rpi::Dma::Ti ti,Rpi::Bus::Memory *src,uint32_t sofs,Rpi::Bus::Address dst,uint32_t nbytes)
{
  auto section = src->phys(sofs) ;
  if (section.second < nbytes)
    throw std::runtime_error("PhysMem:access out of range") ;
  ctl->write(ti,section.first,dst,nbytes,0) ;
}

