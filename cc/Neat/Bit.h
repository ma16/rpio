// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_Bit_h_
#define _Neat_Bit_h_

#include <limits>
#include <type_traits>

namespace Neat {

template<unsigned P,typename U> struct Bit
{
  using Unsigned = U ;

  static auto const pos = P ;

  static_assert(pos < std::numeric_limits<Unsigned>::digits ,"out of bound") ;

  static auto const mask = static_cast<Unsigned>(1) << pos ;
  
  Unsigned get() const { return (*p) & mask ; }
	
  void clear() { (*p) &= ~mask ; }
  void raise() { (*p) |=  mask ; }

  void set(bool b) { if (b) raise() ; else clear() ; }

  Bit& operator=(bool b) { set(b) ; return (*this) ; }
  
  Bit(Unsigned *p) : p(p) {}
  
private:
    
  Unsigned *p ; 
  
} ;

}

#endif // _Neat_Bit_h_
