// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_cast_h_
#define _Neat_cast_h_

#include "Error.h"
#include <memory> // shared_ptr
#include <type_traits> // std::make_unsigned

namespace Neat
{
  template<typename Unsigned> typename std::make_signed<Unsigned>::type to_signed(Unsigned u)
  {
    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<unsigned>::value,"unsigned type required") ;
    auto i = static_cast<typename std::make_signed<Unsigned>::type>(u) ;
    if (i < 0)
      throw Neat::Error("cast:to_signed:unsigned too big") ;
    return i ;
  }

  template<typename To,typename From> To demote(From from)
  {
    auto to = static_cast<To>(from) ;
    if (from != static_cast<From>(to))
      throw Neat::Error("demote failed") ;
    return to ;
  }

  template<typename T> typename std::make_signed<T>::type as_signed(T i)
  {
    static_assert(std::is_integral<T>::value,"integral type required") ;
    static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
    return static_cast<typename std::make_signed<T>::type>(i) ;
  }

  template<typename T> typename std::make_unsigned<T>::type as_unsigned(T i)
  {
    static_assert(std::is_integral<T>::value,"integral type required") ;
    static_assert(std::is_signed  <T>::value,  "signed type required") ;
    return static_cast<typename std::make_unsigned<T>::type>(i) ;
  }

  template<typename E> constexpr typename std::underlying_type<E>::type as_base(E e)
  {
    static_assert(std::is_enum<E>::value,"enum type required") ;
    return static_cast<typename std::underlying_type<E>::type>(e) ;
  }

  template<typename T> inline T volatile* as_volatile(T *p) { return const_cast<T volatile*>(p) ; } 
  template<typename T> inline T volatile& as_volatile(T &p) { return const_cast<T volatile&>(p) ; }
  
  template<typename T> inline T* clip_volatile(T volatile*p) { return const_cast<T*>(p) ; } 
  template<typename T> inline T& clip_volatile(T volatile&p) { return const_cast<T&>(p) ; } 

  template<typename T> inline T const* as_const(T *p) { return const_cast<T const*>(p) ; } 
  template<typename T> inline T const& as_const(T &p) { return const_cast<T const&>(p) ; } 
 
  template<typename T> inline T* clip_const(T const*p) { return const_cast<T*>(p) ; } 
  template<typename T> inline T& clip_const(T const&p) { return const_cast<T&>(p) ; } 

  template<typename T> inline std::shared_ptr<T> clip_const(std::shared_ptr<T const> p) { return std::const_pointer_cast<T>(p) ; } 


  template<typename T,typename F> T promote(F f) ;
  // [todo] make compile time checked
  template<> inline   signed long promote<  signed long,  signed int>(  signed int i) { return i ; }
  template<> inline unsigned long promote<unsigned long,unsigned int>(unsigned int i) { return i ; }

  template<typename T,typename F> T demote(F f) ;
  // [todo] make compile time checked
  template<> inline   signed int demote<  signed int,  signed long>(  signed long i) { auto d = static_cast<  signed int>(i) ; if (i != d) throw Neat::Error("cast:demote") ; return d ; }
  template<> inline unsigned int demote<unsigned int,unsigned long>(unsigned long i) { auto d = static_cast<unsigned int>(i) ; if (i != d) throw Neat::Error("cast:demote") ; return d ; }

}

#endif // _Neat_cast_h_
