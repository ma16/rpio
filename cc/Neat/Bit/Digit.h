// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Digit_h
#define INCLUDE_Neat_Bit_Digit_h

#include "Mask.h"
#include "Word.h"
#include <limits>

namespace Neat { namespace Bit {
	
template<typename U,unsigned O> struct Digit
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"") ;
    static_assert(std::is_unsigned<Unsigned>::value,"") ;
    
    static constexpr auto Offset = O ;
    
    static_assert(Offset<std::numeric_limits<Unsigned>::digits,"") ;

    using Mask = Neat::Bit::Mask<Unsigned,Unsigned(1u << Offset)> ;

    using Word = Neat::Bit::Word<Unsigned,Mask::Bits> ;
    
    constexpr static auto On = Word(Mask::Bits) ;

} ; } }
	
#endif // INCLUDE_Neat_Bit_Digit_h
