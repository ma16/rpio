// BSD 2-Clause License, see github.com/ma16/rpio

// safely access a (32-bit) word that holds pre-masked bit-flags

#ifndef INCLUDE_Neat_Bit_Word_h
#define INCLUDE_Neat_Bit_Word_h

#include "Digit.h"
#include "Sequence.h"

namespace Neat { namespace Bit {

template<typename U,U M> struct Word
{
    // [todo] is Word the right class name?
    
    using Unsigned = U ;

    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;

    static constexpr auto Mask = M ;
    
    static constexpr bool Tight = ~Mask == 0 ; // all bits used

    // ---- native type -----------------------------------------------
    
    template<Unsigned i> static constexpr Word make()
    {
	static_assert(i == (i & Mask),"") ; return i ;
    }
    
    template<typename U2=Unsigned> 
    constexpr Word(typename std::enable_if<Tight,U2>::type i) : i(i) {}
    
    template<unsigned M2> constexpr Word(Word<U,M2> w) : i(w.i)
    {
	static_assert(Mask == (Mask | M2),"") ;
    }

    static Word coset(Unsigned i) { return i & Mask ; }

    constexpr Unsigned value() const { return i ; }

    // ---- bit operator ----------------------------------------------
    
    constexpr Word operator~ () const { return i ^ Mask ; }

    constexpr bool test(Word w) const { return 0 != (this->i & w.i) ; }
    
    // ---- Digit -----------------------------------------------------

    template<unsigned O> constexpr Word(Digit<U,O>) : i(Digit<U,O>::Mask)
    {
	static_assert(Mask == (Mask | Digit<U,O>::Mask),"") ;
    }

    template<unsigned O> bool test(Digit<U,O>) const
    {
	static constexpr auto Mask2 = Digit<U,O>::Mask ;
	static_assert(Mask == (Mask | Mask2),"") ;
	return 0 != (this->i & Mask2) ;
    }
    
    template<unsigned O> Word& operator+=(Digit<U,O>)
    {
	static constexpr auto Mask2 = Digit<U,O>::Mask ;
	static_assert(Mask == (Mask | Mask2),"") ;
	this->i |= Mask2 ;
	return (*this) ;
    }

    template<unsigned O> Word& operator-=(Digit<U,O>)
    {
	static constexpr auto Mask2 = Digit<U,O>::Mask ;
	static_assert(Mask == (Mask | Mask2),"") ;
	this->i &= ~Mask2 ;
	return (*this) ;
    }

    // ---- Sequence --------------------------------------------------
    
    template<unsigned O,unsigned L> Word& operator%=(Sequence<U,O,L> set)
    {
	static constexpr auto Mask2 = Sequence<U,O,L>::Mask ;
	static_assert(Mask == (Mask | Mask2),"") ;
	this->i &= ~Mask2 ;
	this->i |= set.value() ;
	return (*this) ;
    }
    
private:

    template<typename U2,U2 M2> friend class Word ;
    
    Unsigned i ; 

    template<typename U2=Unsigned> 
    constexpr Word(typename std::enable_if<!Tight,U2>::type i) : i(i) {}
    
} ; } }

#endif // INCLUDE_Neat_Bit_Word_h
