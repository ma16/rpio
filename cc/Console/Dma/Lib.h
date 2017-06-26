// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Dma_Lib_h_
#define _Console_Dma_Lib_h_

// --------------------------------------------------------------------
// command line arguments
// --------------------------------------------------------------------

#include <Neat/cast.h>
#include <Rpi/Bus/Memory.h>
#include <Rpi/Dma.h>
#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console { namespace Dma { namespace Lib
{
  Rpi::Dma::Cs optCs(Ui::ArgL *argL,Rpi::Dma::Cs cs) ;

  Rpi::Dma::Ti optTi(Ui::ArgL *argL,Rpi::Dma::Ti ti) ;

  struct Control 
  {
    Control(Rpi::Bus::Memory::shared_ptr mem) : mem_(mem) {}
    // ...mem must be 32-byte aligned and hold a multiple of 32-byte blocks
  
    void write(Rpi::Dma::Ti ti,Rpi::Bus::Address src,Rpi::Bus::Address dst,uint32_t nbytes,uint32_t stride) ;
    // ...throws if the given memory (in the c'tor) is exhausted

    void repeat(size_t ofs) ;

    Rpi::Bus::Memory* mem() { assert(i>0) ; return mem_.get() ; }
    // ...the physical memory address to be ued by Rpi::Dma

    Rpi::Bus::Address addr() { assert(i>0) ; return Rpi::Bus::Address(mem_->phys(0).first) ; }
  
    static std::string toStr(Rpi::Bus::Memory *cb) ;
    
  private:
  
    Rpi::Bus::Memory::shared_ptr mem_ ; uint32_t i=0 ;
  } ;
  
  void write(Control *ctl,Rpi::Dma::Ti ti,Rpi::Bus::Address src,Rpi::Bus::Memory *dst,uint32_t dofs,uint32_t nbytes) ;

  void write(Control *ctl,Rpi::Dma::Ti ti,Rpi::Bus::Memory *src,uint32_t sofs,Rpi::Bus::Address dst,uint32_t nbytes) ;
  
} } } 

#endif // _Console_Dma_Lib_h_
