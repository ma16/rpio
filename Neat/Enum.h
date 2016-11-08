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

// --------------------------------------------------------------------
// An unsigned integer value in the range 0..max
// --------------------------------------------------------------------

#ifndef _Neat_Enum_h_
#define _Neat_Enum_h_

#include "Error.h"
#include <limits>
#include <sstream> 

namespace Neat
{
  template<typename D,D M> struct Enum
  {
    using Domain = D ;
    
    static_assert(std::is_integral<Domain>::value,"integral type required") ;
    static_assert(std::is_unsigned<Domain>::value,"unsigned type required") ;

    static Domain const max = M ;

    // ---- c'tor ----
    
    constexpr Enum() : i(0) {}

    // ---- make ----
    
    template<Domain i> constexpr static Enum make() 
    { static_assert(i<=max,"out of range") ; return Enum(i) ; }

    template<typename T> static Enum make(T i)
    {
      static_assert(std::is_integral<T>::value,"integral type required") ;
      static_assert(std::is_unsigned<T>::value,"unsigned type required") ;
      if (i <= max)
	return Enum(static_cast<Domain>(i)) ;
      std::ostringstream os ;
      os << "Enum:" << std::to_string(i) << " out of range (0," << std::to_string(max) << ')' ;
      // ...to_string() promotes if domain=char
      throw Error(os.str()) ;
    }

    // ---- access ----
    
    constexpr Domain value() const { return i ; }

    // ---- iterate ----
    
    static Enum first() { return 0 ; } 

    bool next() { if (i == max) return false ; ++i ; return true ; }

    // ---- verify ----
    
    template<Domain i> constexpr static void static_check() 
    { static_assert(i<=max,"out of range") ; }
    
  private:

    constexpr Enum(Domain i) : i(i) {}

    Domain i ;
  } ;
}

#endif // _Neat_Enum_h_
