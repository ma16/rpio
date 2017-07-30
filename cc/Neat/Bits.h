// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Neat_Bits_h
#define INCLUDE_Neat_Bits_h

namespace Neat
{

template<uint32_t M> struct Bits
{
    static constexpr auto Mask = M ;
    
    static Bits coset(uint32_t w) { return Bits(Mask & w) ; }

    template<uint32_t W> static constexpr Bits make()
    {
	static_assert((W | Mask) == Mask,"") ; return Bits(W) ;
    }

    template<uint32_t W> constexpr bool test() const
    {
	return test(make<W>()) ;
    }
    
    constexpr bool test(Bits s) const
    {
	return 0 != (w & s.w) ;
    }
	
    void add(Bits c) { w |=         c.w ; } // raise
    
    void clr(Bits c) { w &= Mask & ~c.w ; } // clear
	
    void set(Bits mask,Bits c)
    {
	w &= Mask & ~mask.w ;
	w |=            c.w ;
    }
	
    constexpr uint32_t value() const { return w ; }

private:

    uint32_t w ; constexpr Bits(uint32_t w) : w(w) {}
	
} ; }

#endif // INCLUDE_Neat_Bits_h
