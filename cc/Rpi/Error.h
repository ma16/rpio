// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Error_h_
#define _Rpi_Error_h_

#include <Neat/Error.h>

namespace Rpi 
{
  struct Error : Neat::Error
  {
    Error(std::string const &s) : Neat::Error("Rpi:" + s) {}
  } ;
} 

#endif // _Rpi_Error_h_
