// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Ui_Error_h_
#define _Ui_Error_h_

#include <Neat/Error.h>

namespace Ui 
{
  struct Error : Neat::Error
  {
    Error(std::string const &s) : Neat::Error("Ui:" + s) {}
  } ;
} 

#endif // _Ui_Error_h_
