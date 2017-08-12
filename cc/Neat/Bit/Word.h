// BSD 2-Clause License, see github.com/ma16/rpio

// safely access a (32-bit) word that holds pre-masked bit-flags

#ifndef INCLUDE_Neat_Bit_Word_h
#define INCLUDE_Neat_Bit_Word_h

#include <limits>
#include <type_traits>
#include <stdexcept>
#include "../Error.h"

namespace Neat { namespace Bit {

template<typename U,U M> struct Word
{
    using Unsigned = U ;

    static constexpr auto Digits = std::numeric_limits<Unsigned>::digits ;
    
    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;

    static constexpr auto Mask = M ;
    
    static Word coset(Unsigned w) { return Word(Mask & w) ; }

    template<Unsigned W> static constexpr Word make()
    {
	static_assert((W | Mask) == Mask,"out of range") ; return Word(W) ;
    }

    struct Digit
    {
	template<unsigned Offset> static constexpr Digit make()
	{
	    static_assert(Offset < Word::Digits,"too large") ; return Offset ;
	}

	unsigned value() const { return i ; }

	Word word() const { return Word(1u << i) ; }
	
    private:

	unsigned i ; constexpr Digit(unsigned i) : i(i) {}
    } ;

    bool test(Digit d) const
    {
	return 0 != (d.word().i & i) ;
    }

    template<unsigned O> struct Bit
    {
	static constexpr auto Offset = O ;
	
	static_assert(Offset < Digits,"too large") ;
	
	static constexpr auto Mask = (1u << Offset) ;

	static_assert(Mask == (Mask & Word::Mask),"doesn't match") ;
	
	static constexpr auto Digit = Word::Digit::make<Offset>() ;

	static constexpr Bit make(bool b)
	{
	    return (static_cast<Unsigned>(b) << Offset) ;
	}
	
	bool value() const { return 0 != (i >> Offset) ; }

	constexpr Word word() const { return Word(i) ; }
	
	constexpr Bit(Word w) : i(w.i & Mask) {}

    private:
	
	Unsigned i ; constexpr Bit(Unsigned i) : i(i) {}
    } ;

    template<unsigned Offset> Word& operator=(Bit<Offset> bit)
    {
	this->i &= ~bit.Mask ;
	this->i |=  bit.word().i ;
	return (*this) ;
    }

    template<unsigned O,unsigned L> struct Set
    {
	static constexpr auto Offset = O ;
	static constexpr auto Len    = L ;
	
	static constexpr auto Mask =
	    (~Unsigned(0) << Offset) ^ (~Unsigned(0) << (Offset+Len)) ;
	//static_assert(Mask == (Mask & Word::Mask),"out of range") ;
	
	template<Unsigned I>static constexpr Set make()
	{
	    static_assert(I == (I & (Mask>>Offset)),"out of range") ;
	    return (I << Offset) ;
	}

	static Set make(Unsigned i)
	{
	    if (i != (i & (Mask>>Offset)))
		throw Neat::Error("Neat::Word::Set:out of range") ;
	    return (i << Offset) ;
	}

	Unsigned value() const { return i ; }

	constexpr Set(Word w) : i((w.i & Mask) >> Offset) {}
    
    private:

	Unsigned i ; constexpr Set(unsigned i) : i(i) {}

    } ;

    template<unsigned Offset,unsigned Len> Word& operator=(Set<Offset,Len> set)
    {
	this->i &= ~set.Mask ;
	this->i |=  set.value() ;
	return (*this) ;
    }

    constexpr Unsigned value() const { return i ; }

    constexpr Word() : i(0) {}
  
private:

    Unsigned i ; constexpr Word(Unsigned i) : i(i) {}
	
} ; } }

#endif // INCLUDE_Neat_Bit_Word_h
