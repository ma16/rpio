// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Literal_h
#define INCLUDE_Neat_Literal_h

namespace Neat {

template<typename T,T V> struct Literal
{
    using Type = T ; constexpr static auto Value = V ;
} ;

}

#endif // INCLUDE_Neat_Literal_h
