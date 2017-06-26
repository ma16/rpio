// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Rpi_Bus_Alloc_h
#define Rpi_Bus_Alloc_h

#include "Address.h"
#include "../Mbox/Memory.h"

namespace Rpi { namespace Bus {

struct Alloc
{
  struct Chunk
  {
    Bus::Address address() const { return address_ ; }

    size_t nbytes() const { return nbytes_ ; }
    
    template<typename P> P as() const
    {
      static_assert(std::is_pointer<P>::value,"not a pointer") ;
      auto p = Neat::clip_const(this)->p ;
      return reinterpret_cast<P>(p) ;
      // [todo] should only return const pointer !?
    }
  
    template<typename P> P as()
    {
      auto p = Neat::as_const(this)->as<P>() ; return Neat::clip_const(p) ;
    }

  private:
    
    friend Alloc ;

    Chunk(void *p,Bus::Address address,size_t nbytes) : p(p),address_(address),nbytes_(nbytes) {}

    void *p ; Bus::Address address_ ; size_t nbytes_ ;
  } ;

  static Alloc reserve(size_t nbytes) ;

  Chunk seize(size_t nbytes,size_t align=sizeof(uint32_t)) ;

  template<typename T> Chunk seize(T const v,size_t align=sizeof(T))
  {
    auto chunk = seize(sizeof(T),align) ;
    new (chunk.p) T(v) ;
    return chunk ;
  }

  Mbox::Memory::shared_ptr memory() const { return p ; }
	    
private:

  Mbox::Memory::shared_ptr p ;
  
  size_t ofs ;
  
  Alloc(Mbox::Memory::shared_ptr p,size_t ofs) : p(p), ofs(ofs) {}

} ;

} }

#endif // Rpi_Bus_Alloc_h
