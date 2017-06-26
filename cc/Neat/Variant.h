// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_Variant_h_
#define _Neat_Variant_h_

#include <type_traits>

namespace Neat
{
  template<typename T,typename... More> struct Variant
  {
    template<typename U> constexpr bool has() const { return std::is_same<T,U>::value || Variant<More...>().has<U>() ; }    
  } ;

  template<typename T> struct Variant<T>
  {
    template<typename U> constexpr bool has() const { return std::is_same<T,U>::value ; }
  } ;
}

#endif // _Neat_Variant_h_
