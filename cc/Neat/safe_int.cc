// BSD 2-Clause License, see github.com/ma16/rpio

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
  os << "safe_int:" << std::to_string(a) << '+' << std::to_string(b) << " exceeds " << std::to_string(std::numeric_limits<T>::max()) ;
  throw Neat::Error(os.str()) ;
}

template<>               bool Neat::safe_t<              bool>::operator+ (              bool x) const { return add(this->u,x) ; }
template<> unsigned      char Neat::safe_t<unsigned      char>::operator+ (unsigned      char x) const { return add(this->u,x) ; }
template<> unsigned     short Neat::safe_t<unsigned     short>::operator+ (unsigned     short x) const { return add(this->u,x) ; }
template<> unsigned       int Neat::safe_t<unsigned       int>::operator+ (unsigned       int x) const { return add(this->u,x) ; }
template<> unsigned      long Neat::safe_t<unsigned      long>::operator+ (unsigned      long x) const { return add(this->u,x) ; }
template<> unsigned long long Neat::safe_t<unsigned long long>::operator+ (unsigned long long x) const { return add(this->u,x) ; }

template<typename T> static T mult(T a,T b)
{
  if (b == 0)
    return 0 ;
  if (a <= std::numeric_limits<T>::max() / b)
    return static_cast<T>(a * b) ;
  // ...cast since auto promotion to int for char and short
  std::ostringstream os ;
  os << "safe_int:" << std::to_string(a) << '*' << std::to_string(b) << " exceeds " << std::to_string(std::numeric_limits<T>::max()) ;
  throw Neat::Error(os.str()) ;
}

template<>               bool Neat::safe_t<              bool>::operator* (              bool x) const { return mult(this->u,x) ; }
template<> unsigned      char Neat::safe_t<unsigned      char>::operator* (unsigned      char x) const { return mult(this->u,x) ; }
template<> unsigned     short Neat::safe_t<unsigned     short>::operator* (unsigned     short x) const { return mult(this->u,x) ; }
template<> unsigned       int Neat::safe_t<unsigned       int>::operator* (unsigned       int x) const { return mult(this->u,x) ; }
template<> unsigned      long Neat::safe_t<unsigned      long>::operator* (unsigned      long x) const { return mult(this->u,x) ; }
template<> unsigned long long Neat::safe_t<unsigned long long>::operator* (unsigned long long x) const { return mult(this->u,x) ; }

