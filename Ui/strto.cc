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

#include "strto.h"

#include "Error.h"

#include <cstring> // index()
#include <limits> // std::numeric_limits
#include <sstream>

static unsigned long long make_ull(char const *s)
{
  if (index(s,'-') != nullptr) {
    // ...stroull says that negative values are considered valid input
    // and are silently converted to the equivalent unsigned value.
    std::ostringstream os ;
    os << "strto(" << s << "):must not be signed" ;
    throw Ui::Error(os.str()) ;
  }
  char *p ;
  errno = 0 ;
  auto i = strtoull(s,&p,0) ;
  if ((errno!=0) || p[0]) {
    std::ostringstream os ;
    os << "strto(" << s << "):invalid unsigned integer value" ;
    throw Ui::Error(os.str()) ;
  }
  return i ;
}

template<> unsigned long long Ui::strto(char const *s) { return make_ull(s) ; }

template<typename T> static T make_unsigned(char const *s)
{
  auto i = make_ull(s) ;
  if (i > std::numeric_limits<T>::max()) {
    std::ostringstream os ;
    os << "strto(" << s << "):out of integer range (0," << std::numeric_limits<T>::max() << ')' ;
    throw Ui::Error(os.str()) ;
  }
  return static_cast<T>(i) ;
}

template<>           bool Ui::strto(char const *s) { return make_unsigned<          bool>(s) ; }
template<> unsigned  char Ui::strto(char const *s) { return make_unsigned<unsigned  char>(s) ; }
template<> unsigned short Ui::strto(char const *s) { return make_unsigned<unsigned short>(s) ; }
template<> unsigned   int Ui::strto(char const *s) { return make_unsigned<unsigned   int>(s) ; }
template<> unsigned  long Ui::strto(char const *s) { return make_unsigned<unsigned  long>(s) ; }

static signed long long make_ll(char const *s)
{
  char *p ;
  errno = 0 ;
  auto i = strtoll(s,&p,0) ;
  if ((errno!=0) || p[0]) {
    std::ostringstream os ;
    os << "strto(" << s << "):invalid signed integer value" ;
    throw Ui::Error(os.str()) ;
  }
  return i ;
}

template<> signed long long Ui::strto(char const *s) { return make_ll(s) ; }

template<typename T> static T make_signed(char const *s)
{
  auto i = make_ll(s) ;
  if (i < std::numeric_limits<T>::min() || std::numeric_limits<T>::max() < i) {
    std::ostringstream os ;
    os << "strto(" << s << "):out of integer range (" << std::numeric_limits<T>::min() << ',' << std::numeric_limits<T>::max() << ')' ;
    throw Ui::Error(os.str()) ;
  }
  return static_cast<T>(i) ;
}

template<> signed  char Ui::strto(char const *s) { return make_signed<signed  char>(s) ; }
template<> signed short Ui::strto(char const *s) { return make_signed<signed short>(s) ; }
template<> signed   int Ui::strto(char const *s) { return make_signed<signed   int>(s) ; }
template<> signed  long Ui::strto(char const *s) { return make_signed<signed  long>(s) ; }

static long double make_ld(char const *s)
{
  char *p ;
  errno = 0 ;
  auto i = strtold(s,&p) ;
  if ((errno!=0) || p[0]) {
    std::ostringstream os ;
    os << "strto(" << s << "):invalid floating point value" ;
    throw Ui::Error(os.str()) ;
  }
  return i ;
}

template<> long double Ui::strto(char const *s) { return make_ld(s) ; }

template<typename T> static T make_floating(char const *s)
{
  auto i = make_ld(s) ;
  if ((i < std::numeric_limits<T>::lowest()) || (std::numeric_limits<T>::max() < i)) {
    std::ostringstream os ;
    os << "strto(" << s << "):out of floating point range (" << std::numeric_limits<T>::lowest() << ',' << std::numeric_limits<T>::max() << ')' ;
    throw Ui::Error(os.str()) ;
  }
  return static_cast<T>(i) ;
}

template<> double Ui::strto(char const *s) { return make_floating<double>(s) ; }
template<>  float Ui::strto(char const *s) { return make_floating< float>(s) ; }

template<> Neat::NotSigned<signed      char> Ui::strto(char const *s) { return Neat::NotSigned<signed      char>::make(strto<unsigned      char>(s)) ; }
template<> Neat::NotSigned<signed     short> Ui::strto(char const *s) { return Neat::NotSigned<signed     short>::make(strto<unsigned     short>(s)) ; }
template<> Neat::NotSigned<signed       int> Ui::strto(char const *s) { return Neat::NotSigned<signed       int>::make(strto<unsigned       int>(s)) ; }
template<> Neat::NotSigned<signed      long> Ui::strto(char const *s) { return Neat::NotSigned<signed      long>::make(strto<unsigned      long>(s)) ; }
template<> Neat::NotSigned<signed long long> Ui::strto(char const *s) { return Neat::NotSigned<signed long long>::make(strto<unsigned long long>(s)) ; }

