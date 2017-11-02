// BSD 2-Clause License, see github.com/ma16/rpio

// safely access a (32-bit) word that holds pre-masked bit-flags

#ifndef INCLUDE_Neat_Bit_Word_h
#define INCLUDE_Neat_Bit_Word_h

#include "Mask.h"
#include <limits>

namespace Neat { namespace Bit {

template<typename U,U B> struct Word
{
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;

    using Mask = Neat::Bit::Mask<Unsigned,B> ; // [todo] do we really need mask??

    static Word coset(Unsigned i) { return i & Mask::Bits ; }

    constexpr Unsigned value() const { return i ; }

    constexpr Word operator~ () const { return i ^ Mask::Bits ; }
    
    template<unsigned B2> 
    constexpr Word<U,B|B2> operator| (Word<U,B2> w) const
    {
	return Word<U,B|B2>(i | w.i) ;
    }
    
    template<unsigned B2> 
    constexpr Word<U,B|B2> operator& (Word<U,B2> w) const
    {
	return Word<U,B|B2>(i & w.i) ;
    }
    
    template<unsigned B2> constexpr Word(Word<U,B2> w) : i(w.i)
    {
	static_assert(B == (B | B2),"") ;
    }

    template<typename U2=Unsigned> 
    constexpr Word(typename std::enable_if<Mask::Tight,U2>::type i) : i(i) {}
    
private:

    template<typename U2,unsigned O2> friend class Digit ;

    template<typename U2,U2 B2> friend class Word ;
    
    Unsigned i ; 

    template<typename U2=Unsigned> 
    constexpr Word(typename std::enable_if<!Mask::Tight,U2>::type i) : i(i) {}
    
} ; } }

#endif // INCLUDE_Neat_Bit_Word_h
