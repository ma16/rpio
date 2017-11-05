// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bit_Sequence_h
#define INCLUDE_Neat_Bit_Sequence_h

#include "../uint.h" // [todo] move here

namespace Neat { namespace Bit {
	
template<typename U,unsigned O,unsigned L> struct Sequence
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;
    
    static constexpr auto Offset = O ;
    static constexpr auto Len    = L ;

    static_assert(Offset <  std::numeric_limits<Unsigned>::digits,"") ;
    static_assert(Len    >  0,"") ;
    static_assert(Len    <= std::numeric_limits<Unsigned>::digits,"") ;
    static_assert(Offset <= std::numeric_limits<Unsigned>::digits-Len,"") ;
    
    static constexpr auto Mask = Unsigned((~ (~Unsigned(0) << Len) ) << Offset) ;

    // ---- native bit-set type ---------------------------------------
    
    static Sequence coset(Unsigned i) { return i & Mask ; }
    
    constexpr Unsigned value() const { return i ; }
    
    // ---- integral number type --------------------------------------
    
    template<Unsigned i> static constexpr Sequence make()
    {
	static_assert(i == (i & (Mask >> Offset)),"") ;
	return i << Offset ;
    }

    constexpr Unsigned count() const { return i >> Offset ; }

    // ---- Uint<> type -----------------------------------------------
  
    using Uint = Neat::uint<Unsigned,Len> ;

    constexpr Sequence(Uint uint) : i(uint.value() << Offset) {}

private:

    Unsigned i ; constexpr Sequence(unsigned i) : i(i) {}
  
} ; } }
    
#endif // INCLUDE_Neat_Bit_Sequence_h
