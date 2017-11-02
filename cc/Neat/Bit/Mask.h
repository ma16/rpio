// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Mask_h
#define INCLUDE_Neat_Bit_Mask_h

#include <type_traits>

namespace Neat { namespace Bit {
	
template<typename U,U B> struct Mask
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;

    static constexpr auto Bits = B ;

    static constexpr bool Tight = Bits == Unsigned(~0u) ;
    
} ; } }

#endif // INCLUDE_Neat_Bit_Mask_h
