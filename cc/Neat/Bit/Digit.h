// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Digit_h
#define INCLUDE_Neat_Bit_Digit_h

// A literal that points to the N-th bit inside a word.
//
// The word must be one of: unsigned char, short, int, long or long long

#include <limits>
#include <type_traits>

namespace Neat { namespace Bit {

template<typename U,unsigned O> struct Digit
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"") ;
    static_assert(std::is_unsigned<Unsigned>::value,"") ;
    
    static constexpr auto Offset = O ;
    
    static_assert(Offset<std::numeric_limits<Unsigned>::digits,"") ;

    static constexpr auto Mask = (U)((U)1 << O) ;

} ; } }
	
#endif // INCLUDE_Neat_Bit_Digit_h
