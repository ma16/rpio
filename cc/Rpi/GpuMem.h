// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Rpi_GpuMem_h
#define Rpi_GpuMem_h

#include "Bus/Memory.h"
#include "Mbox/Memory.h"
#include <cassert>

namespace Rpi {

struct GpuMem : Bus::Memory
{
  static shared_ptr allocate(Mbox::Interface::shared_ptr iface,
			     uint32_t                   nbytes,
			     uint32_t                    align,
			     Mbox::Property::Memory::Mode mode)
    
  { return std::shared_ptr<GpuMem>(new GpuMem(Mbox::Memory::allocate(iface,nbytes,align,mode))) ; }

  virtual void* virt() override { return p->front() ; }
  
  virtual std::pair<Bus::Address,size_t> phys(size_t ofs) override
  {
    assert(ofs <= p->size()) ;
    return std::make_pair(Bus::Address(p->address().value()+ofs),p->size()-ofs) ;
  }

  virtual size_t nbytes() const override { return p->size() ; }

  ~GpuMem() override { }

  struct Allocator : Bus::Memory::Allocator
  {
    virtual GpuMem::shared_ptr allocate(size_t nbytes) override { return GpuMem::allocate(iface,nbytes,align,mode) ; }

    Allocator(Mbox::Interface::shared_ptr iface,uint32_t align,Mbox::Property::Memory::Mode mode)
      : iface(iface),align(align),mode(mode) {}

  private:

    Mbox::Interface::shared_ptr iface ;
    
    uint32_t align ;

    Mbox::Property::Memory::Mode mode ;
  } ;

private:

  Mbox::Memory::shared_ptr p ; GpuMem(Mbox::Memory::shared_ptr p) : p(p) {}

} ;

}


#endif // Rpi_GpuMem_h
