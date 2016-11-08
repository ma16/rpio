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

// --------------------------------------------------------------------
// get/set word at any peripheral address
// --------------------------------------------------------------------

#include "../rpio.h"
#include <Rpi/Peripheral.h>
#include <Ui/strto.h>
#include <iostream>

namespace Main { namespace Poke {

void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE\n"
	      << '\n'
	      << "MODE : get ADDR\n"
	      << "     | set ADDR WORD\n"
	      << '\n'
	      << "ADDR : peripheral address offset: 0..FFFFFC\n"
	      << "WORD : 32-bit integer number\n"
	      << '\n'
	      << "For example, the GPIO input levels are located at offset 0x200034.\n"
	      << std::flush ;
    return ;
  }
  std::string mode = argL->pop() ;
  auto addr = Ui::strto<size_t>(argL->pop()) ;
  if (mode == "get") {
    argL->finalize() ;
    std::cout << std::hex << rpi->at(addr) << std::endl ;
  }
  else if (mode == "set") {
    auto word = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    rpi->at(addr) = word ;
  }
  else throw std::runtime_error("not supported option:<"+mode+'>') ;
}

} /* Poke */ } /* Main */
