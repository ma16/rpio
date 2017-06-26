// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_NotSigned_h_
#define _Neat_NotSigned_h_

#include <limits>
#include <type_traits>

namespace Neat
{
  template<typename T> struct NotSigned 
  {
    using Signed = T ;

    static_assert(std::is_integral<Signed>::value,"integral type required") ;
    static_assert(std::is_signed  <Signed>::value,  "signed type required") ;
    
    using Unsigned = typename std::make_unsigned<Signed>::type ;
    
    static auto const max = static_cast<Unsigned>(std::numeric_limits<Signed>::max()) ;

    static bool isValid(Unsigned u) { return u <= max ; }
    //static bool isValid(  Signed i) { return i >=   0 ; }

    static NotSigned make(Unsigned u) ;
    static NotSigned make(  Signed i) { return make(static_cast<Unsigned>(i)) ; }

    template<Unsigned u> static const NotSigned make() 
    { static_assert(u <= max,"out of range") ; return NotSigned(u) ; }

    constexpr   Signed   as_signed() const { return static_cast<Signed>(u) ; }
    constexpr Unsigned as_unsigned() const { return                     u  ; }

    constexpr NotSigned() : u(0) {}

    template<typename B> constexpr NotSigned(NotSigned<B> u) : u(u.u) {}
    // [todo] make B-to-T promote-able compile-time-checked
    
  private:

    explicit constexpr NotSigned(Unsigned u_) : u(u_) {}

    Unsigned u ;
  } ;

  template<> NotSigned<signed      char> NotSigned<signed      char>::make(unsigned      char u) ;
  template<> NotSigned<signed     short> NotSigned<signed     short>::make(unsigned     short u) ;
  template<> NotSigned<signed       int> NotSigned<signed       int>::make(unsigned       int u) ;
  template<> NotSigned<signed      long> NotSigned<signed      long>::make(unsigned      long u) ;
  template<> NotSigned<signed long long> NotSigned<signed long long>::make(unsigned long long u) ;

}

#endif // _Neat_NotSigned_h_
