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

#include "safe_int.h"
#include "Error.h"
#include <limits>
#include <sstream>

template<typename T> static T add(T a,T b)
{
  if (a <= std::numeric_limits<T>::max() - b)
    return static_cast<T>(a + b) ;
  // ...cast since auto promotion to int for char and short
  std::ostringstream os ;
  os << "safe_add(" << std::to_string(a) << ',' << std::to_string(b) << ") exceeds " << std::to_string(std::numeric_limits<T>::max()) ;
  throw Neat::Error(os.str()) ;
}

template<> bool               Neat::safe_add(bool               a,bool               b) { return add(a,b) ; }
template<> unsigned      char Neat::safe_add(unsigned      char a,unsigned      char b) { return add(a,b) ; }
template<> unsigned     short Neat::safe_add(unsigned     short a,unsigned     short b) { return add(a,b) ; }
template<> unsigned       int Neat::safe_add(unsigned       int a,unsigned       int b) { return add(a,b) ; }
template<> unsigned      long Neat::safe_add(unsigned      long a,unsigned      long b) { return add(a,b) ; }
template<> unsigned long long Neat::safe_add(unsigned long long a,unsigned long long b) { return add(a,b) ; }

template<typename T> static T mult(T a,T b)
{
  if (a <= std::numeric_limits<T>::max() / b)
    // [todo] watch div by zero
    return static_cast<T>(a * b) ;
  // ...cast since auto promotion to int for char and short
  std::ostringstream os ;
  os << "safe_mult(" << std::to_string(a) << ',' << std::to_string(b) << ") exceeds " << std::to_string(std::numeric_limits<T>::max()) ;
  throw Neat::Error(os.str()) ;
}

template<> bool               Neat::safe_mult(bool               a,bool               b) { return mult(a,b) ; }
template<> unsigned      char Neat::safe_mult(unsigned      char a,unsigned      char b) { return mult(a,b) ; }
template<> unsigned     short Neat::safe_mult(unsigned     short a,unsigned     short b) { return mult(a,b) ; }
template<> unsigned       int Neat::safe_mult(unsigned       int a,unsigned       int b) { return mult(a,b) ; }
template<> unsigned      long Neat::safe_mult(unsigned      long a,unsigned      long b) { return mult(a,b) ; }
template<> unsigned long long Neat::safe_mult(unsigned long long a,unsigned long long b) { return mult(a,b) ; }

