// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_Error_h_
#define _Neat_Error_h_

#include <stdexcept>

namespace Neat
{
  struct Error : std::runtime_error
  {
    Error(std::string const &s) : std::runtime_error("Neat:" + s) {}
    // remember that the message-string-creation might cause a
    // "premature" exception (e.g. bad_alloc) here, or in the
    // runtime_error constructor or in the client code that creates
    // an exception message. -> [opt] "prepare" exception messages
    // (e.g. with static storage) and use std::exception as base.
  } ;
} 

#endif // _Neat_Error_h_
