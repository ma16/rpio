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

#include "Error.h"
#include "NotSigned.h"
#include <sstream> 

template<typename T> static T check(T u) 
{
  using Class = typename Neat::NotSigned<typename std::make_signed<T>::type> ;
  if (!Class::isValid(u)) {
    std::ostringstream os ;
    os << u << " not in range (0," << Class::max << ')' ;
    throw Neat::Error("NotSigned:" + os.str()) ;
  }
  return u ;
}

template<> Neat::NotSigned<signed      char> Neat::NotSigned<signed      char>::make(unsigned      char u) { return NotSigned(check(u)) ; }
template<> Neat::NotSigned<signed     short> Neat::NotSigned<signed     short>::make(unsigned     short u) { return NotSigned(check(u)) ; }
template<> Neat::NotSigned<signed       int> Neat::NotSigned<signed       int>::make(unsigned       int u) { return NotSigned(check(u)) ; }
template<> Neat::NotSigned<signed      long> Neat::NotSigned<signed      long>::make(unsigned      long u) { return NotSigned(check(u)) ; }
template<> Neat::NotSigned<signed long long> Neat::NotSigned<signed long long>::make(unsigned long long u) { return NotSigned(check(u)) ; }
