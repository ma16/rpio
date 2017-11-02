// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Set_h
#define INCLUDE_Neat_Bit_Set_h

#include "Mask.h"
#include "../uint.h"
#include <limits>

namespace Neat { namespace Bit {
	
template<typename U,unsigned O,unsigned L> struct Set
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;
    
    static constexpr auto Offset = O ;
    static constexpr auto Len    = L ;

    static_assert(Offset < std::numeric_limits<Unsigned>::digits,"") ;
    static_assert(Len    > 0,"") ;
    static_assert(Len    < std::numeric_limits<Unsigned>::digits,"") ;
    static_assert(Offset < std::numeric_limits<Unsigned>::digits-Len,"") ;
    
    using Uint = Neat::uint<Unsigned,Len> ;

    static constexpr auto helper = Unsigned((~ (~Unsigned(0) << Len) ) << Offset) ;
    
    using Mask = Neat::Bit::Mask<Unsigned,helper> ;
  
} ; } }
    
#endif // INCLUDE_Neat_Bit_Set_h
