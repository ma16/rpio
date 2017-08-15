// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_uint_h_
#define _Neat_uint_h_

// --------------------------------------------------------------------
// for example:
// using uint24_t = Neat::uint<uint32_t,24> ;
// --------------------------------------------------------------------

#include "Error.h"
#include <limits>
#include <type_traits>

namespace Neat {

template<typename U,unsigned D> struct uint 
{
  using Unsigned = U ;

  static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
  static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;
    
  static const auto digits = D ;
    
  static_assert((0 < digits) && (digits <= std::numeric_limits<Unsigned>::digits),"out of range") ;

  static unsigned mask() { return ~(~U(0) << digits) ; }
  
  static const auto max = (1u << digits) -1 ;
    
  template<Unsigned u> static bool isValid() { return u <= max ; }

  static bool isValid(Unsigned u) { return u <= max ; }

  template<Unsigned u> constexpr static uint make()
  {
    static_assert(u<=max,"out of range") ; return u ;
  }

  static uint make(Unsigned u)
  {
    if (u<=max) return u ;
    throw Neat::Error("uint:out of range") ;
  }

  static uint coset(Unsigned u) { return mask() & u ; }

  constexpr Unsigned value() const { return u ; }

  constexpr uint() : u(0) {}

private:

  Unsigned u ; constexpr uint(Unsigned u) : u(u) {}

} ; }

#endif // _Neat_uint_h_
