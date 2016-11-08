// Copyright (c) 2016, "ma16". All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.

// This project is hosted at https://github.com/ma16/rpio

#ifndef _Neat_Numerator_h_
#define _Neat_Numerator_h_

// --------------------------------------------------------------------
// Sometimes you want to iterate throw a C++11-enum, or simply address
// an enum by its unique integer value. However, there appears to be no
// safe way to do so. This Numerator<> class may help.
//
// E.g.
//   enum class Color : unsigned char { Red=0 , Blue=1 , Green=2 } ;
//   using ColorN = Neat::Numerator<Color,Green> ;
//
// As you see, there are certain constraints, though:
// -- N enums must be in the range 0..N-1
// -- the value <N-1> must be provided as template argument
// -- the underlying_type must be unsigned
// --------------------------------------------------------------------

#include "Error.h"
#include <limits>
#include <sstream>
#include <type_traits> // underlying_type

namespace Neat
{
  template<typename E,E M> struct Numerator 
  {
    using Enum = E ;

    static_assert(std::is_enum<Enum>::value,"enum type required") ;

    using Domain = typename std::underlying_type<Enum>::type ;
    
    static_assert(std::is_unsigned<Domain>::value,"unsigned underlying_type required") ;

    static auto const max = static_cast<Domain>(M) ;

    // ---- c'tor ----
    
    constexpr Numerator() : i(0) {}

    constexpr Numerator(Enum e) : i(static_cast<Domain>(e)) {}
    
    // ---- make ----
    
    template<Domain i> constexpr static Numerator make()
    { static_assert(i<=max,"out of range") ; return Numerator(i) ; }

    template<typename T> static Numerator make(T i)
    {
      static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
      if (i <= max)
	return Numerator(static_cast<Domain>(i)) ;
      std::ostringstream os ;
      os << "Numerator:" << std::to_string(i) << " out of range (0," << std::to_string(max) << ')' ;
      // ...to_string() promotes if domain=char
      throw Error(os.str()) ;
    }

    // ---- access ----
    
    constexpr Enum e() const { return static_cast<Enum>(i) ; }
    
    constexpr Domain n() const { return i ; }

    // ---- iterate ----
    
    static Numerator first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i> constexpr static void static_check() 
    { static_assert(i<=max,"out of range") ; }
    
  private:
    
    Domain i ; constexpr Numerator(Domain i) : i(i) {}

  } ;
}

#endif // _Neat_Numerator_h_
