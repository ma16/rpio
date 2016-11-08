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

#include "../rpio.h"
#include "Host.h"
#include <Neat/cast.h>
#include <Ui/strto.h>
#include <chrono>
#include <deque>
#include <iostream>

namespace Main { namespace Mcp3008 {

static void rateInvoke(Host *host,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: N  # the number of consecutive samples to take\n"
	      << std::flush ;
    return ;
  }
  auto n = Ui::strto<unsigned>(argL->pop()) ;
  argL->finalize() ;
  auto t0 = std::chrono::steady_clock::now() ;
  decltype(n) i ;
  for (i=0 ; i<n ; ++i) {
    auto sample = host->query(Host::Source::make<0x8>()) ;
    if (!sample) {
      std::cout << "Error <" << Neat::as_base(*host->error()) << "> at sample <" << i << '>' << std::endl ;
      break ;
    }
  }
  auto t1 = std::chrono::steady_clock::now() ;
  std::cout.setf(std::ios::scientific) ;
  std::cout.precision(2) ;
  std::cout << i/std::chrono::duration<double>(t1-t0).count() << "/s" << std::endl ;
}
    
static void sampleInvoke(Host *host,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: SOURCE+\n"
	      << '\n'
	      << "SOURCE+ : SOURCE | SOURCE SOURCE+\n"
	      << '\n'
	      << "SOURCE :  0  # differential CH0 = IN+ CH1 = IN-\n"
	      << "       |  1  # differential CH0 = IN- CH1 = IN+\n"
	      << "       |  2  # differential CH2 = IN+ CH3 = IN-\n"
	      << "       |  3  # differential CH2 = IN- CH3 = IN+\n"
	      << "       |  4  # differential CH4 = IN+ CH5 = IN-\n"
	      << "       |  5  # differential CH4 = IN- CH5 = IN+\n"
	      << "       |  6  # differential CH6 = IN+ CH7 = IN-\n"
	      << "       |  7  # differential CH6 = IN- CH7 = IN+\n"
	      << "       |  8  # single-ended CH0\n"
	      << "       |  9  # single-ended CH1\n"
	      << "       | 10  # single-ended CH2\n"
	      << "       | 11  # single-ended CH3\n"
	      << "       | 12  # single-ended CH4\n"
	      << "       | 13  # single-ended CH5\n"
	      << "       | 14  # single-ended CH6\n"
	      << "       | 15  # single-ended CH7\n"
	      << std::flush ;
    return ;
  }
  auto source = Ui::strto(argL->pop(),Host::Source()) ;
  std::deque<decltype(source)> q(1,source) ;
  while (!argL->empty()) {
    q.push_back(Ui::strto(argL->pop(),Host::Source())) ;
  }
  argL->finalize() ;
  for (auto source: q) {
    auto sample = host->query(source) ;
    if (!sample) {
      std::cout << "Error <" << Neat::as_base(*host->error()) << "> while sampling" ;
      break ;
    }
    std::cout << sample->value() << " " ;
  }
  std::cout << std::endl ;
}
    
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") {
    std::cout << "arguments: CS CLK DIN DOUT [-m] MODE [help]\n"
	      << '\n'
	      << "  CS: Pi's pin to feed MCP's CS pin\n"
	      << " CLK: Pi's pin to feed MCP's CLK pin\n"
	      << " DIN: Pi's pin to feed MCP's DIN pin\n"
	      << "DOUT: Pi's pin to read MCP's DOUT pin\n"
	      << '\n'
	      << "-m: enable monitoring to detect communication errors\n"
	      << '\n'
	      << "MODE :   rate  # perform throughput test\n"
	      << "     | sample  # read one or more samples\n"
	      << std::flush ;
    return ;
  }
  
  auto   csPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto  clkPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto   toPin = Ui::strto(argL->pop(),Rpi::Pin()) ;
  auto fromPin = Ui::strto(argL->pop(),Rpi::Pin()) ;

  auto monitor = argL->pop_if("-m") ;
  
  Host host(rpi,csPin,clkPin,toPin,fromPin,Host::Timing(),monitor) ;
  // ...[future] make timings optional arguments

  auto arg = argL->pop() ;
  if      (arg ==   "rate")   rateInvoke(&host,argL) ;
  else if (arg == "sample") sampleInvoke(&host,argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Mcp3008 */ } /* Main */
