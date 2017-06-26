// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_PhysMem_h_
#define _Neat_PhysMem_h_

#include "cast.h"
#include <type_traits> // std::is_pointer		
#include <utility> // std::pair

namespace Neat { struct PhysMem
{
  using shared_ptr = std::shared_ptr<PhysMem> ;
  
  virtual void* virt() = 0 ;
  // ...contiguous area of virtual memory (p,p+n]

  template<typename P> P as() const
  {
    static_assert(std::is_pointer<P>::value,"not a pointer") ;
    auto p = clip_const(this)->virt() ;
    return reinterpret_cast<P>(p) ;
  }
  
  template<typename P> P as()
  {
    auto p = as_const(this)->as<P>() ; return clip_const(p) ;
  }
  
  virtual std::pair<uintptr_t,size_t> phys(size_t ofs) = 0 ;
  // ...memory block (p,p+m] at given offset
  
  virtual size_t nbytes() const = 0 ;
  // ...allocated memory size

  virtual ~PhysMem() {}

  struct Factory
  {
    using shared_ptr = std::shared_ptr<Factory> ;
    
    virtual PhysMem::shared_ptr allocate(size_t nbytes) = 0 ;

    virtual ~Factory() {}
  } ;
  
} ; }

#endif // _Neat_PhysMem_h_
