// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Posix_MMap_h_
#define _Posix_MMap_h_

#include "Fd.h"
#include <Neat/cast.h>
#include <sys/mman.h> // PROT_READ...

namespace Posix
{
  struct MMap
  {
    enum class Prot { RO=PROT_READ,WO=PROT_WRITE,RW=PROT_READ|PROT_WRITE } ;

    using shared_ptr = std::shared_ptr<MMap> ;
    
    static shared_ptr make(Fd *fd,Fd::uoff_t offset,size_t length,Prot prot,bool share) ; 
    // ...note: offset must be a multiple of the page-size
    
    void* front() const { return p ; }
    size_t size() const { return n ; }
      
    template<typename P> P as() const
    {
      static_assert(std::is_pointer<P>::value,"not a pointer") ;
      return reinterpret_cast<P>(this->front()) ;
    }
  
    template<typename P> P as()
    {
      auto p = Neat::as_const(this)->as<P>() ; return Neat::clip_const(p) ;
    }
  
    ~MMap() ;
      
  private:
      
    void *p ; size_t n ; MMap(void *p,size_t n) : p(p),n(n) {}

    // notes:
    // --mmap can be either on regular file, device file or anonymous
    // --a mapping that is not covered by (enough) physical memory
    //   may cause a SIGSEGV on access (zero page fault). An initial
    //   memset(3)+signal catcher and/or mlock(2) might help to verify
    //   beforehand.
  } ;
} 

#endif // _Posix_MMap_h_
