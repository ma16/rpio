// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_RpiExt_VcMem_h
#define INCLUDE_RpiExt_VcMem_h

#include <Rpi/Bus/Memory.h>
#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace RpiExt { namespace VcMem
{
  using Memory = Rpi::Bus::Memory ;
  
  Memory::Allocator::shared_ptr defaultAllocator() ;

} } 

#endif // INCLUDE_RpiExt_VcMem_h
