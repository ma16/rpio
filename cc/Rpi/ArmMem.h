// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_ArmMem_h_
#define _Rpi_ArmMem_h_

#include "Bus/Memory.h" 
#include <Neat/PhysMem.h>

namespace Rpi { struct ArmMem : Bus::Memory
{
  static shared_ptr allocate(size_t nbytes,Rpi::Bus::Coherency,bool stick) ;
    // --coherency (prefix) to be or'ed to the physical ARM address
    // --heap allocation or persistent shared memory segment (stick)

  virtual void* virt() override ;
  
  virtual std::pair<Bus::Address,size_t> phys(size_t ofs) override ;
  
  virtual size_t nbytes() const override ;

  struct Factory : Bus::Memory::Factory
  {
    virtual ArmMem::shared_ptr allocate(size_t nbytes) override ;

    Factory(Rpi::Bus::Coherency co,bool stick) : co(co),stick(stick) {}
    // since allocate() takes only size as argument, we have to have
    // coherency and sticky (shared memory segment) to set up here

  private:

    Rpi::Bus::Coherency co ; bool stick ;
  } ;

private:

  ArmMem(Neat::PhysMem::shared_ptr mem,Rpi::Bus::Coherency co) :  mem(mem),co(co) {}
  
  Neat::PhysMem::shared_ptr mem ; Rpi::Bus::Coherency co ; 
} ; }

#endif // _Rpi_ArmMem_h_
