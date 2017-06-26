// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Bus_Memory_h_
#define _Rpi_Bus_Memory_h_

// --------------------------------------------------------------------
// -work-in-progress-
//
// An Area of Memory that can be accessed by the ARM and the Video Core
//
// From the ARM point of view (userland) it is a contiguoues block of
// virtual memory.
//
// From the ARM point of view (physical) it is a list of physical pages
// which may be physcially contiguous, but most probably are not (at
// least not if allocated through sbrk from userland).
//
// From the VC point of view it is a list of physical pages which may
// be physcially contiguous (if allocated by the VC via Mailbox
// interface) or not (if allocated by an ARM userland process by sbrk).
// The VC core uses physical address for cache-coherency (plus a few
// other properties if allocated thru Mailbox interface).
//
// A main problem with this type of shared memory (besides the
// segmentation of physical pages) is the coherency. That is, when/how
// become changes visible to the other core. It appears to be unsafe
// to allocate thru ARM/sbrk [refer to use-case]. Though, there seems
// to be kernel methods to get coherent memory (maybe there is a
// reserved MMU region?). On the other hand, memory allocated by the
// VideoCore (thru Mailbox interface) appears to be always coherent and
// besides, also physically contiguous (which is a real benefit).
// --------------------------------------------------------------------


#include "Address.h"
#include <Neat/cast.h>
#include <memory> // std::shared_ptr
#include <type_traits> // std::is_pointer		
#include <utility> // std::pair

namespace Rpi { namespace Bus { struct Memory
{
  using shared_ptr = std::shared_ptr<Memory> ;
  
  virtual void* virt() = 0 ;
  // ...contiguous area of virtual memory (p,p+n]

  template<typename P> P as() const
  {
    static_assert(std::is_pointer<P>::value,"not a pointer") ;
    auto p = Neat::clip_const(this)->virt() ;
    return reinterpret_cast<P>(p) ;
  }
  
  template<typename P> P as()
  {
    auto p = Neat::as_const(this)->as<P>() ; return Neat::clip_const(p) ;
  }
  
  virtual std::pair<Bus::Address,size_t> phys(size_t ofs) = 0 ;
  // ...memory block (p,p+m] at given offset
  
  virtual size_t nbytes() const = 0 ;
  // ...allocated memory size

  virtual ~Memory() {}

  struct Factory
  {
    using shared_ptr = std::shared_ptr<Factory> ;
    
    virtual Memory::shared_ptr allocate(size_t nbytes) = 0 ;

    virtual ~Factory() {}
  } ;
  
} ; } }

#endif // _Rpi_Bus_Memory_h_
