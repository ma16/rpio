// BSD 2-Clause License, see github.com/ma16/rpio

// safely access a (32-bit) word that holds pre-masked bit-flags

#ifndef INCLUDE_Neat_Bit_WordOld_h
#define INCLUDE_Neat_Bit_WordOld_h

#include "../uint.h"
#include <limits>
#include <type_traits>

namespace Neat { namespace Bit {
	
template<typename U,U M> struct WordOld
{
    using Unsigned = U ;

    static constexpr auto Digits = std::numeric_limits<Unsigned>::digits ;
    
    static_assert(std::is_integral<Unsigned>::value,"integral type required") ;
    static_assert(std::is_unsigned<Unsigned>::value,"unsigned type required") ;

    static constexpr auto Mask = M ;
    
    static WordOld coset(Unsigned w) { return WordOld(Mask & w) ; }

    template<Unsigned W> static constexpr WordOld make()
    {
	static_assert((W | Mask) == Mask,"out of range") ; return WordOld(W) ;
    }

    struct Digit
    {
	template<unsigned Offset> static constexpr Digit make()
	{
	    static_assert(Offset < WordOld::Digits,"too large") ; return Offset ;
	}

	unsigned value() const { return i ; }

	WordOld word() const { return WordOld(1u << i) ; }
	
    private:

	unsigned i ; constexpr Digit(unsigned i) : i(i) {}
    } ;

    bool test(Digit d) const
    {
	return 0 != (d.word().i & i) ;
    }

    WordOld& operator-=(Digit d)
    {
	this->i &= ~d.word().i ;
	return (*this) ;
    }

    WordOld& operator+=(Digit d)
    {
	this->i |= d.word().i ;
	return (*this) ;
    }

    template<unsigned O> struct Bit
    {
	static constexpr auto Offset = O ;
	
	static_assert(Offset < Digits,"too large") ;
	
	static constexpr auto Mask = (1u << Offset) ;

	static_assert(Mask == (Mask & WordOld::Mask),"doesn't match") ;
	
	static constexpr auto Digit = WordOld::Digit::template make<Offset>() ;

	constexpr Bit(WordOld w) : i(w.i & Mask) {}

	static constexpr Bit make(bool b)
	{
	    return (static_cast<Unsigned>(b) << Offset) ;
	}
	
	bool raised() const { return 0 != i ; }

	constexpr WordOld word() const { return WordOld(i) ; }
	
    private:
	
	Unsigned i ; constexpr Bit(Unsigned i) : i(i) {}
    } ;

    template<unsigned Offset> WordOld& operator%=(Bit<Offset> bit)
    {
	this->i &= ~bit.Mask ;
	this->i |=  bit.word().i ;
	return (*this) ;
    }

    template<unsigned O,unsigned L> struct Set
    {
	static constexpr auto Offset = O ;
	static constexpr auto Len    = L ;

	using Uint = Neat::uint<Unsigned,Len> ;
	
	static constexpr auto Mask = (~ (~Unsigned(0) << Len) ) << Offset ;
	
	static_assert(Mask == (Mask & WordOld::Mask),"out of range") ;

	constexpr Set(Uint uint) : i(uint.value() << Offset) {}

	constexpr Set(WordOld w) : i(w.i & Mask) {}

	template<Unsigned I> static constexpr Set make()
	{
	    static_assert(I == (I & (Mask >> Offset)),"out of range") ;
	    return I << Offset ;
	}

	constexpr Set() : i(0) {}
	
	constexpr Unsigned count() const { return i >> Offset ; }

	constexpr WordOld word() const { return WordOld(i) ; }
	
    private:

	Unsigned i ; constexpr Set(unsigned i) : i(i) {}

    } ;

    template<unsigned O,unsigned L> Set<O,L> test(Set<O,L>) const
    {
	return Set<O,L>(*this) ;
    }

    template<unsigned Offset,unsigned Len> WordOld& operator%=(Set<Offset,Len> set)
    {
	this->i &= ~set.Mask ;
	this->i |=  set.word().i ;
	return (*this) ;
    }

    constexpr Unsigned value() const { return i ; }

    constexpr WordOld() : i(0) {}
  
private:

    Unsigned i ; constexpr WordOld(Unsigned i) : i(i) {}

} ; } }

#endif // INCLUDE_Neat_Bit_WordOld_h
