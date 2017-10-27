// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_Numerator_h_
#define _Neat_Numerator_h_

// --------------------------------------------------------------------
// Sometimes you want to iterate throw a C++11-enum, or simply address
// an enum by its unique integer value. However, there appears to be no
// safe way to do so. This Numerator<> class may help.
//
// E.g.
//   enum class Color : unsigned char { Red=0 , Blue=1 , Green=2 } ;
//   using ColorN = Neat::Numerator<Color,Green> ;
//
// As you see, there are certain constraints, though:
// -- N enums must be in the range 0..N-1
// -- the value <N-1> must be provided as template argument
// -- the underlying_type must be unsigned
// --------------------------------------------------------------------

#include "Error.h"
#include <limits>
#include <sstream>
#include <type_traits> // underlying_type

namespace Neat
{
  template<typename E,E M> struct Numerator 
  {
    using Enum = E ;

    static_assert(std::is_enum<Enum>::value,"enum type required") ;

    using Domain = typename std::underlying_type<Enum>::type ;
    
    static_assert(std::is_unsigned<Domain>::value,"unsigned underlying_type required") ;

    static auto const max = static_cast<Domain>(M) ;

    // ---- c'tor ----
    
    constexpr Numerator() : i(0) {}

    constexpr Numerator(Enum e) : i(static_cast<Domain>(e)) {}
    
    // ---- make ----
    
    template<Domain i> constexpr static Numerator make()
    { static_assert(i<=max,"out of range") ; return Numerator(i) ; }

    template<typename T> static Numerator make(T i)
    {
      static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
      if (i <= max)
	return Numerator(static_cast<Domain>(i)) ;
      std::ostringstream os ;
      os << "Numerator:" << std::to_string(i) << " out of range (0," << std::to_string(max) << ')' ;
      // ...to_string() promotes if domain=char
      throw Error(os.str()) ;
    }

    static Numerator coset(Domain i) { return Numerator(i%(max+1)) ; }
    // [todo] max+1 == 0
				       
    // ---- access ----
    
    constexpr Enum e() const { return static_cast<Enum>(i) ; }
    
    constexpr Domain n() const { return i ; }

    // ---- iterate ----
    
    static Numerator first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i> constexpr static void static_check() 
    { static_assert(i<=max,"out of range") ; }
    
  private:
    
    Domain i ; constexpr Numerator(Domain i) : i(i) {}

  } ;
}

#endif // _Neat_Numerator_h_
