// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_Literal_h_
#define _Neat_Literal_h_

#include "Error.h"
#include <limits>
#include <sstream> 

namespace Neat { template<typename T,T V> struct Literal
{
  using Type = T ; constexpr static auto value = V ;
} ; }

#endif // _Neat_Literal_h_
