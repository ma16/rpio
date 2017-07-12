// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Pwm_Lib_h_
#define _Console_Pwm_Lib_h_

#include <Console/Dma/Lib.h>
#include <Rpi/Pwm.h>

namespace Console { namespace Pwm { namespace Lib
{
  struct Error : Neat::Error { Error(std::string const &s) : Neat::Error("Console:Pwm:Lib:" + s) {} } ;

  unsigned send(Rpi::Pwm pwm,Rpi::Pwm::Index index,uint32_t const data[],unsigned nwords) ;
  // ...undefined bahavior if the other channel runs in fifo mode too
  // ...effects pwen,mode,usef
  // ...caller should set range,sbit,pola & rptl
  
  void setup(Rpi::Pwm *pwm,Rpi::Pwm::Index index) ;

  void start(Rpi::Pwm *pwm,Rpi::Pwm::Index index) ;

  void finish(Rpi::Pwm *pwm,Rpi::Pwm::Index index) ;
  
} } }

#endif // _Console_Pwm_Lib_h_
