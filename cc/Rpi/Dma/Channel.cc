// BSD 2-Clause License, see github.com/ma16/rpio

#include "Channel.h"
#include <iomanip> // setw
#include <sstream>

void Rpi::Dma::Channel::setup(Bus::Address cb,Cs cs)
{
  reset() ;
  Debug d ;
  d.rerror() = true ;
  d.ferror() = true ;
  d.lerror() = true ;
  setDebug(d) ;
  setCs(cs) ;
  setCb(cb) ;
}

std::string Rpi::Dma::Channel::toStr() const
{
  std::ostringstream os ;
  os << std::hex 
     << std::setw(8) << this->p[0] 
     << std::setw(9) << this->p[1] 
     << std::setw(9) << this->p[2] 
     << std::setw(9) << this->p[3] 
     << std::setw(9) << this->p[4] 
     << std::setw(9) << this->p[5] 
     << std::setw(9) << this->p[6] 
     << std::setw(9) << this->p[7] 
     << std::setw(9) << this->p[8] ;
  return os.str() ;
}
