// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_safe_int_h_
#define _Neat_safe_int_h_

#include <limits>
#include <type_traits>

namespace Neat
{
  template<typename T> struct safe_t
  {
    static_assert(std::is_integral<T>::value,"integral type required") ;
    static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
    T operator+ (T x) const ;
    T operator* (T x) const ;
    T u ; constexpr explicit safe_t(T u) : u(u) {} ; 
  } ;

  template<typename T> constexpr inline safe_t<T> make_safe(T u) { return safe_t<T>(u) ; }
  
  template<>               bool safe_t<              bool>::operator+ (              bool x) const ;
  template<>               bool safe_t<              bool>::operator* (              bool x) const ;
  template<> unsigned      char safe_t<unsigned      char>::operator+ (unsigned      char x) const ;
  template<> unsigned      char safe_t<unsigned      char>::operator* (unsigned      char x) const ;
  template<> unsigned     short safe_t<unsigned     short>::operator+ (unsigned     short x) const ;
  template<> unsigned     short safe_t<unsigned     short>::operator* (unsigned     short x) const ;
  template<> unsigned       int safe_t<unsigned       int>::operator+ (unsigned       int x) const ;
  template<> unsigned       int safe_t<unsigned       int>::operator* (unsigned       int x) const ;
  template<> unsigned      long safe_t<unsigned      long>::operator+ (unsigned      long x) const ;
  template<> unsigned      long safe_t<unsigned      long>::operator* (unsigned      long x) const ;
  template<> unsigned long long safe_t<unsigned long long>::operator+ (unsigned long long x) const ;
  template<> unsigned long long safe_t<unsigned long long>::operator* (unsigned long long x) const ;
  
  template<typename A,typename B> using unsigned_promote = std::conditional<(std::numeric_limits<A>::max()>std::numeric_limits<B>::max()),A,B> ;
  
  template<typename A,typename B> inline typename unsigned_promote<A,B>::type safe_add(A a,B b)
  {
    using T = typename unsigned_promote<A,B>::type ;
    return make_safe(static_cast<T>(a)) + static_cast<T>(b) ;
  }
  
  template<typename A,typename B> inline typename unsigned_promote<A,B>::type safe_mult(A a,B b)
  {
    using T = typename unsigned_promote<A,B>::type ;
    return make_safe(static_cast<T>(a)) * static_cast<T>(b) ;
  }
}

#endif // _Neat_safe_int_h_
