// BSD 2-Clause License, see github.com/ma16/rpio

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
