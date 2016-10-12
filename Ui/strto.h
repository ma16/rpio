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

#ifndef _Ui_strto_h_
#define _Ui_strto_h_

#include <Neat/Enum.h>
#include <Neat/NotSigned.h>
#include <string>

namespace Ui
{
  template<typename T> T strto(char const *s) ;
  // ...only the specializations below are implemented
  // ...[opt] template<typename T> enable_if<supported_by_strto<T>,T> strto(char const *s) ;

  template<> bool strto<bool>(char const *s) ;

  template<> signed        char strto(char const *s) ;
  template<> unsigned      char strto(char const *s) ;
  template<> signed       short strto(char const *s) ;
  template<> unsigned     short strto(char const *s) ;
  template<> signed         int strto(char const *s) ;
  template<> unsigned       int strto(char const *s) ;
  template<> signed        long strto(char const *s) ;
  template<> unsigned      long strto(char const *s) ;
  template<> signed   long long strto(char const *s) ;
  template<> unsigned long long strto(char const *s) ;

  template<>       float strto(char const *s) ;
  template<>      double strto(char const *s) ;
  template<> long double strto(char const *s) ;

  template<> Neat::NotSigned<signed      char> strto(char const *s) ;
  template<> Neat::NotSigned<signed     short> strto(char const *s) ;
  template<> Neat::NotSigned<signed       int> strto(char const *s) ;
  template<> Neat::NotSigned<signed      long> strto(char const *s) ;
  template<> Neat::NotSigned<signed long long> strto(char const *s) ;

  template<typename T> inline T strto(std::string const &s) { return strto<T>(s.c_str()) ; }
  // ...std::string wrapper

  template<typename D,D M> Neat::Enum<D,M> strto(char const *s,Neat::Enum<D,M>)
  // ...the 2nd argument is (only) for the sake of auto template deduction
  { return Neat::Enum<D,M>::make(Ui::strto<D>(s)) ; }

  template<typename D,D M> Neat::Enum<D,M> strto(std::string const &s,Neat::Enum<D,M> e) { return strto(s.c_str(),e) ; }
  // ...std::string wrapper
}

#endif // _Ui_strto_h_
