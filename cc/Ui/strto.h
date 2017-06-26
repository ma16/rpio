// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Ui_strto_h_
#define _Ui_strto_h_

#include <Neat/Enum.h>
#include <Neat/NotSigned.h>
#include <Neat/uint.h>
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

  template<typename T> Neat::NotSigned<T> strto(char const *s,Neat::NotSigned<T>)
  // ...the 2nd argument is (only) for the sake of auto template deduction
  { return Neat::NotSigned<T>::make(Ui::strto<typename Neat::NotSigned<T>::Unsigned>(s)) ; }

  template<typename T> Neat::NotSigned<T> strto(std::string const &s,Neat::NotSigned<T> u) { return strto(s.c_str(),u) ; }
  // ...std::string wrapper

  template<typename U,unsigned D> Neat::uint<U,D> strto(char const *s,Neat::uint<U,D>)
  // ...the 2nd argument is (only) for the sake of auto template deduction
  { return Neat::uint<U,D>::make(Ui::strto<U>(s)) ; }

  template<typename U,unsigned D> Neat::uint<U,D> strto(std::string const &s,Neat::uint<U,D> u) { return strto(s.c_str(),u) ; }
  // ...std::string wrapper
}

#endif // _Ui_strto_h_
