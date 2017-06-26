// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Enable & disable interrupts
// --------------------------------------------------------------------

#include "../rpio.h"
#include <Rpi/Intr.h>
#include <Ui/strto.h>
#include <iostream>

static Rpi::Intr::Vector make(bool gpu,unsigned irq)
{
  Rpi::Intr::Vector v(0,0,0) ;
  if (gpu) {
    if (irq < 32) {
      v.lo = (1u << irq) ;
    }
    else if (irq < 64) {
      v.hi = (1u << (irq-32)) ;
    }
    else throw Ui::Error("GPU-IRQ <" + std::to_string(irq) + "> too big") ;
  }
  else {
    if (irq < 8) {
      v.base = static_cast<decltype(v.base)>(1u << irq) ;
    }
    else throw Ui::Error("ARM-IRQ <" + std::to_string(irq) + "> too big") ;
  }
  return v ;
}

static void status(Rpi::Intr::Vector v)
{
  std::cout << "GPU:" ;
  for (auto i=0 ; i<32 ; ++i) {
    if (1 & v.lo)
      std::cout << ' ' << i ;
    v.lo >>= 1 ;
  }
  for (auto i=0 ; i<32 ; ++i) {
    if (1 & v.hi)
      std::cout << ' ' << (i+32) ;
    v.hi >>= 1 ;
  }
  std::cout << '\n' ;
  std::cout << "ARM:" ;
  for (auto i=0 ; i<8 ; ++i) {
    if (1 & v.base)
      std::cout << ' ' << i ;
    v.base = static_cast<decltype(v.base)>(v.base >> 1) ;
  }
  std::cout << std::endl ;
}


void Console::Intr::invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE [help]\n"
	      << '\n'
	      << "MODE : disable ARM | GPU\n"
      	      << "     |  enable ARM | GPU\n"
	      << "     |  status\n"
	      << '\n'
	      << " ARM : arm 0.. 7\n"
	      << " GPU : gpu 0..63\n"
	      << std::flush ;
    return ;
  }
  auto mode = argL->pop({"disable","enable","status"}) ;
  if (mode == 2) {
    argL->finalize() ;
    return status(Rpi::Intr(rpi).status()) ;
  }
  auto side = argL->pop({"arm","gpu"}) ;
  auto irq = Ui::strto<unsigned>(argL->pop()) ;
  argL->finalize() ;
  auto v = make(side==1,irq) ;
  if (mode == 0) Rpi::Intr(rpi).disable(v) ;
  else           Rpi::Intr(rpi). enable(v) ;
}
