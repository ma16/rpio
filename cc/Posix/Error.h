// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Posix_Error_h_
#define _Posix_Error_h_

#include <Neat/Error.h>

namespace Posix 
{
  struct Error : Neat::Error
  {
    Error(std::string const &s) : Neat::Error("Posix:" + s) {}
  } ;
} 

#endif // _Posix_Error_h_
