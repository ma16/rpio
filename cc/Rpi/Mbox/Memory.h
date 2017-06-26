// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Rpi_Mbox_Memory_h
#define Rpi_Mbox_Memory_h

#include "Property/Memory.h"
#include "../Bus/Address.h"
#include <Posix/MMap.h>
#include <cassert>

namespace Rpi { namespace Mbox {

struct Memory
{
  using shared_ptr = std::shared_ptr<Memory> ;
  
  static shared_ptr allocate(Mbox::Interface::shared_ptr,
			     uint32_t nbytes,
			     uint32_t align,
			     Mbox::Property::Memory::Mode) ;

  Bus::Address address() const { return address_ ; }

  void* front() const { return mmap->front() ; }

  size_t size() const { return mmap->size() ; }
  
  ~Memory() ;

private:

  std::shared_ptr<Mbox::Interface> iface ;
    
  uint32_t h ;

  Bus::Address address_ ;

  Posix::MMap::shared_ptr mmap ; 

  Memory(std::shared_ptr<Mbox::Interface> iface,
	 uint32_t                             h,
	 Bus::Address                   address,
	 Posix::MMap::shared_ptr           mmap)
    : iface(iface),h(h),address_(address),mmap(mmap) {}
  
} ;

} }

#endif // Rpi_Mbox_Memory_h
