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

#ifndef _Neat_safe_int_h_
#define _Neat_safe_int_h_

#include <limits>
#include <type_traits>

namespace Neat
{
  template<typename T> typename std::enable_if<std::is_unsigned<T>::value,T>::type safe_add(T a,T b) ;
  
  template<> bool               safe_add(bool               a,bool               b) ;
  template<> unsigned      char safe_add(unsigned      char a,unsigned      char b) ;
  template<> unsigned     short safe_add(unsigned     short a,unsigned     short b) ;
  template<> unsigned       int safe_add(unsigned       int a,unsigned       int b) ;
  template<> unsigned      long safe_add(unsigned      long a,unsigned      long b) ;
  template<> unsigned long long safe_add(unsigned long long a,unsigned long long b) ;

  template<typename A,typename B> using unsigned_promote = std::conditional<(std::numeric_limits<A>::max()>std::numeric_limits<B>::max()),A,B> ;
  
  template<typename A,typename B> inline typename unsigned_promote<A,B>::type safe_add(A a,B b)
  {
    using T = typename unsigned_promote<A,B>::type ;
    return safe_add(static_cast<T>(a),static_cast<T>(b)) ;
  }

  template<typename T> typename std::enable_if<std::is_unsigned<T>::value,T>::type safe_mult(T a,T b) ;
  
  template<> bool               safe_mult(bool               a,bool               b) ;
  template<> unsigned      char safe_mult(unsigned      char a,unsigned      char b) ;
  template<> unsigned     short safe_mult(unsigned     short a,unsigned     short b) ;
  template<> unsigned       int safe_mult(unsigned       int a,unsigned       int b) ;
  template<> unsigned      long safe_mult(unsigned      long a,unsigned      long b) ;
  template<> unsigned long long safe_mult(unsigned long long a,unsigned long long b) ;

  template<typename A,typename B> using unsigned_promote = std::conditional<(std::numeric_limits<A>::max()>std::numeric_limits<B>::max()),A,B> ;
  
  template<typename A,typename B> inline typename unsigned_promote<A,B>::type safe_mult(A a,B b)
  {
    using T = typename unsigned_promote<A,B>::type ;
    return safe_mult(static_cast<T>(a),static_cast<T>(b)) ;
  }
}

#endif // _Neat_safe_int_h_
