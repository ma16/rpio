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
//
// This project is hosted at https://github.com/ma16/rpio

#ifndef _Neat_NotSigned_h_
#define _Neat_NotSigned_h_

#include <limits>
#include <type_traits>

namespace Neat
{
  template<typename T> struct NotSigned 
  {
    using Signed = T ;

    static_assert(std::is_integral<Signed>::value,"integral type required") ;
    static_assert(std::is_signed  <Signed>::value,  "signed type required") ;
    
    using Unsigned = typename std::make_unsigned<Signed>::type ;
    
    static auto const max = static_cast<Unsigned>(std::numeric_limits<Signed>::max()) ;

    static bool isValid(Unsigned u) { return u <= max ; }

    static NotSigned make(Unsigned u) ;

    template<Unsigned u> static const NotSigned init() 
    { static_assert(u <= max,"out of range") ; return NotSigned(u) ; }

    constexpr   Signed   as_signed() const { return static_cast<Signed>(u) ; }
    constexpr Unsigned as_unsigned() const { return                     u  ; }

    constexpr NotSigned() : u(0) {}

  private:

    explicit constexpr NotSigned(Unsigned u_) : u(u_) {}

    Unsigned u ;
  } ;

  template<> NotSigned<signed      char> NotSigned<signed      char>::make(unsigned      char u) ;
  template<> NotSigned<signed     short> NotSigned<signed     short>::make(unsigned     short u) ;
  template<> NotSigned<signed       int> NotSigned<signed       int>::make(unsigned       int u) ;
  template<> NotSigned<signed      long> NotSigned<signed      long>::make(unsigned      long u) ;
  template<> NotSigned<signed long long> NotSigned<signed long long>::make(unsigned long long u) ;
}

#endif // _Neat_NotSigned_h_
