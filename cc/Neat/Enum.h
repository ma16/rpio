// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// An unsigned integer value in the range 0..max
// --------------------------------------------------------------------

#ifndef _Neat_Enum_h_
#define _Neat_Enum_h_

#include "Error.h"
#include <limits>
#include <sstream> 

namespace Neat
{
  template<typename D,D M> struct Enum
  {
    using Domain = D ;
    
    static_assert(std::is_integral<Domain>::value,"integral type required") ;
    static_assert(std::is_unsigned<Domain>::value,"unsigned type required") ;

    static Domain const max = M ;

    // ---- c'tor ----
    
    constexpr Enum() : i(0) {}

    template<Domain N> constexpr Enum(Enum<D,N> e) : i(e.value()) { static_assert(N<=M,"out of range") ; }

    // ---- make ----
    
    template<Domain i> constexpr static Enum make() 
    { static_assert(i<=max,"out of range") ; return Enum(i) ; }

    template<typename T> static Enum make(T i)
    {
      static_assert(std::is_integral<T>::value,"integral type required") ;
      static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
      if (i <= max)
	return Enum(static_cast<Domain>(i)) ;
      // [todo] stuff below shouldn't be in the header
      std::ostringstream os ;
      os << "Enum:" << std::to_string(i) << " out of range (0," << std::to_string(max) << ')' ;
      // ...to_string() promotes if domain=char
      throw Error(os.str()) ;
    }

    // ---- access ----
    
    constexpr Domain value() const { return i ; }

    // ---- iterate ----
    
    static Enum first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i> constexpr static void static_check() 
    { static_assert(i<=max,"out of range") ; }
    
  private:

    constexpr Enum(Domain i) : i(i) {}

    Domain i ;
  } ;
}

#endif // _Neat_Enum_h_
