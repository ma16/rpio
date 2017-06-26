// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_BitPtr_h_
#define _Neat_BitPtr_h_

#include "uint.h"

namespace Neat { template<unsigned O,unsigned D,bool R,bool W,typename U> struct BitPtr
{
  using Unsigned = U ;

  static auto const offset = O ;
  static auto const digits = D ;

  static_assert(offset                    < std::numeric_limits<Unsigned>::digits ,"out of bound") ;
  static_assert((0 < digits) && (digits  <= std::numeric_limits<Unsigned>::digits),"out of bound") ;
  static_assert((offset + digits)        <= std::numeric_limits<Unsigned>::digits ,"out of bound") ;
  
  static auto const readable = R ;
  static auto const writable = W ;

  using Uint = Neat::uint<U,D> ;
      
  Unsigned bits() const
  {
    static_assert(readable,"not readable") ;
    return (*p) & (Uint::mask()<<offset) ;
  }
    
  void clear()
  {
    static_assert(writable,"not writable") ;
    (*p) &= ~(Uint::mask()<<offset) ;
  }

  void raise()
  {
    static_assert(writable,"not writable") ;
    (*p) |=  (Uint::mask()<<offset) ;
  }

  BitPtr& operator=(bool b)
  {
    if (b) raise() ;
    else   clear() ;
    return (*this) ;
  }
  
  Uint get() const
  {
    static_assert(readable,"not readable") ;
    return Uint::coset((*p) >> offset)  ;
  }
	
  BitPtr& operator=(Uint i)
  {
    clear() ;
    (*p) |= (i.value() << offset) ;
    return (*this) ;
  }
  
  BitPtr(Unsigned *p) : p(p) {}
  
private:
    
  Unsigned *p ; 
  
} ; }

#endif // _Neat_BitPtr_h_
