// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// An unsigned integer value in the range 0..max
// --------------------------------------------------------------------

#ifndef INCLUDE_Neat_Enum_h
#define INCLUDE_Neat_Enum_h

#include <limits>
#include <type_traits>

namespace Neat {

void Enum_throw(unsigned long long max,unsigned long long i) ;

template<typename D,D M> constexpr bool Enum_isTight()
{
    static_assert(std::is_integral<D>::value,"") ;
    static_assert(std::is_unsigned<D>::value,"") ;
    static_assert(M <= std::numeric_limits<D>::max(),"") ;
    return M == std::numeric_limits<D>::max() ;
}

template<typename D,D M,bool Tight=Enum_isTight<D,M>()> struct Enum ;

template<typename D,D M> struct Enum<D,M,false>
{
    using Domain = D ; static Domain const max = M ;

    static_assert(max < std::numeric_limits<Domain>::max(),"") ;
    
    bool operator==(Enum e) const { return i == e.i ; }

    // ---- construct ----
    
    constexpr Enum() : i(0) {}

    template<Domain M2> constexpr Enum(Enum<D,M2> e)
    : i(e.value()) { static_assert(M2<=max,"out of range") ; }

    // ---- make ----
    
    template<Domain i>
    constexpr static Enum make() { static_assert(i<=max,"") ; return Enum(i) ; }

    /*
    template<typename T=Enum> static typename std::enable_if< tight(),T>::type
    coset(Domain i) { return Enum(i % (max+1)) ; }
    
    template<typename T=Enum> static typename std::enable_if<!tight(),T>::type
    coset(Domain i) { return Enum(i % (max+1)) ; }
    */

    static Enum coset(Domain i) { return Enum(i % (max+1)) ; }
    
    static Enum make(Domain i)
    {
	if (i > max)
	    Enum_throw(max,i) ; 
	return Enum(i) ;
    }

    template<typename D2>
    static Enum make(D2 i) { return Enum(Enum<D2,max>::make(i)) ; }

    // ---- access ----
    
    constexpr Domain value() const { return i ; }

    // ---- iterate ----
    
    static Enum first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i>
    constexpr static void static_check() { static_assert(i<=max,"") ; }
    
  private:

    Domain i ; constexpr Enum(Domain i) : i(i) {}
} ;

template<typename D,D M> struct Enum<D,M,true>
{
    using Domain = D ; static Domain const max = M ;

    static_assert(max == std::numeric_limits<Domain>::max(),"") ;
    
    bool operator==(Enum e) const { return i == e.i ; }

    // ---- construct ----
    
    constexpr Enum() : i(0) {}

    template<Domain M2> constexpr Enum(Enum<D,M2> e)
    : i(e.value()) { static_assert(M2<=max,"out of range") ; }

    // ---- make ----
    
    template<Domain i>
    constexpr static Enum make() { return Enum(i) ; }

    static Enum coset(Domain i) { return Enum(i) ; }
    
    static Enum make(Domain i) { return Enum(i) ; }

    template<typename D2>
    static Enum make(D2 i) { return Enum(Enum<D2,max>::make(i)) ; }

    // ---- access ----
    
    constexpr Domain value() const { return i ; }

    // ---- iterate ----
    
    static Enum first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i>
    constexpr static void static_check() { }
    
  private:

    Domain i ; constexpr Enum(Domain i) : i(i) {}
} ;
    
}

#endif // INCLUDE_Neat_Enum_h
