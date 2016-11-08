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

#include "rpio.h"
#include <Ui/strto.h>
#include <iostream>

static Posix::Fd::uoff_t base_addr(Ui::ArgL *argL)
{
  auto addr = argL->option("--base") ;
  if (addr)
    return Ui::strto<Posix::Fd::uoff_t>(*addr) ;
  if (argL->pop_if("--devtree"))
    return Rpi::Peripheral::by_devtree() ;
  return Rpi::Peripheral::by_cpuinfo() ;
}

int main(int argc,char **argv)
{
  try {
    auto argL = Ui::ArgL::make(argc-1,argv+1) ;

    if (argL.empty() || argL.peek() == "help") { 
      std::cout << "arguments: [BASE] MODE [help]\n"
		<< '\n'
		<< "BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000\n"
		<< "     | --devtree       # use info in /proc/device-tree/soc/ranges\n"
		<< '\n'
		<< "If BASE is not given then the peripheral address is derived from\n"
		<< "the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.\n"
		<< '\n'
		<< "MODE : clock       # r/w clock sources\n"
		<< "     | gpio        # r/w GPIO\n"
		<< "     | max7219     # dot-matrix control for the MAX7219 circuit\n"
		<< "     | mcp3008     # ADC control for the MCP3008 circuit\n"
		<< "     | poke        # r/w any word in peripheral address space\n"
		<< "     | throughput  # i/o and memory performance tests\n"
		<< '\n'
		<< "Use the keyword help for additional information.\n"
		<< std::flush ;
      return 0 ;
    }
    
    auto rpi = Rpi::Peripheral::make(base_addr(&argL)) ;
    std::string arg = argL.pop() ;
  
    if      (arg ==      "clock") Main::     Clock::invoke(rpi.get(),&argL) ;
    else if (arg ==       "gpio") Main::      Gpio::invoke(rpi.get(),&argL) ;
    else if (arg ==    "max7219") Main::   Max7219::invoke(rpi.get(),&argL) ;
    else if (arg ==    "mcp3008") Main::   Mcp3008::invoke(rpi.get(),&argL) ;
    else if (arg ==       "poke") Main::      Poke::invoke(rpi.get(),&argL) ;
    else if (arg == "throughput") Main::Throughput::invoke(rpi.get(),&argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
  }
  catch (std::exception &error) {
    std::cerr << "exception caught:" << error.what() << std::endl ;
  }
  
  return 0 ;
  // [note] keep in mind that exceptions might also occur after returning
  //   from main() while cleaning up static variables
}
