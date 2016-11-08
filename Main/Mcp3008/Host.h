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
// The MCP3008 is a 10-bit ADC with 4x2 channels at a maximum sampling
// rate of 75k/s (2.7V) up to 200k/s (5V). An SPI-like protocol is used
// to communicate with the device. Though, the implementation provided
// here does "bit-banging" based on the specified time parameters.
// 
// Please refer to the MCP3008 data sheet for more details.
//
// Note that this implementation does not achieve the MCP's maximum
// sampling rate since the Pi's peripheral access is rather slow (about
// 50..100ns to read input, 10..20ns to write output and 50..100ns to
// read a clock) which easily sums up to more than 10us per sample.
//
// [future]
// --what are the impacts of process suspension?
// --speedup by polling edges instead of levels?
// --speedup by DMA usage?
// --support Pi's built-in SPI
// --------------------------------------------------------------------

#ifndef _Main_Mcp3008_Host_h_
#define _Main_Mcp3008_Host_h_

#include <Rpi/Peripheral.h>
#include <boost/optional.hpp>

namespace Main { namespace Mcp3008 {

struct Host
{
  // (Single=1/Diff=0,D2,D1,D0)
  using Source = Neat::Enum<unsigned,0xf> ;
  // 0 0 0 0 differential CH0 = IN+ CH1 = IN-
  // 0 0 0 1 differential CH0 = IN- CH1 = IN+
  // 0 0 1 0 differential CH2 = IN+ CH3 = IN-
  // 0 0 1 1 differential CH2 = IN- CH3 = IN+
  // 0 1 0 0 differential CH4 = IN+ CH5 = IN-
  // 0 1 0 1 differential CH4 = IN- CH5 = IN+
  // 0 1 1 0 differential CH6 = IN+ CH7 = IN-
  // 0 1 1 1 differential CH6 = IN- CH7 = IN+
  // 1 0 0 0 single-ended CH0
  // 1 0 0 1 single-ended CH1
  // 1 0 1 0 single-ended CH2
  // 1 0 1 1 single-ended CH3
  // 1 1 0 0 single-ended CH4
  // 1 1 0 1 single-ended CH5
  // 1 1 1 0 single-ended CH6
  // 1 1 1 1 single-ended CH7

  using Sample = Neat::Enum<unsigned,0x3ff> ;
  // ...10-bit digitized sampled value

  boost::optional<Sample> query(Source) ;

  enum class Error {
    StartCSHi   = 0, // CS,Clk and Din are set by the application and read
    StartClkLo  = 1, // back at setup time. There is serious trouble if the
    StartDInLo  = 2, // read data does not correspond with the written one.
      
    StartDOutHi = 4, // MCP's DOut must be Hi after reset
    RespDOutLo  = 5, // MCP's DOut must be Lo when start to respond
    RecvError   = 6, // Received MSB-value does not match received LSB-value
    EndDOutHi   = 7, // MCP's DOut must be Hi after CS disabled

    BledTimeout = 8, // capacitor voltage started to bleed out
  } ;

  boost::optional<Error> error() { return this->error_ ; }
  
  struct Timing // nano seconds
  {
    unsigned csh  = 270 ; // CS Disable Time
    unsigned sucs = 100 ; // CS Fall To First Rising CLK Edge
    unsigned hd   =  50 ; // Data Input Hold Time
    unsigned su   =  50 ; // Data Input Setup Time
    unsigned en   = 200 ; // CLK Fall To Output Enable: 125..200
    unsigned lo   = 125 ; // Clock High Time
    unsigned hi   = 125 ; // Clock High Time
    unsigned do_  = 200 ; // CLK Fall To Output Data Valid: 125..200
    unsigned dis  = 100 ; // CS Rise To Output Disable
    
    unsigned bled = 1E6 ; // maximum time between the end of the sample
    // -period and the time that all 10 data bits have been clocked out
  } ;
    
  Host(Rpi::Peripheral *rpi,
       Rpi::Pin   csPin,
       Rpi::Pin  clkPin,
       Rpi::Pin   toPin, // DIN
       Rpi::Pin fromPin, // DOUT
       Timing const &t,
       bool monitor=true) ;

private:
  
  Rpi::Peripheral *rpi ;

  Rpi::Pin csPin,clkPin,toPin,fromPin ;

  Timing t ;

  boost::optional<Error> error_ ;

  bool monitor ;

  unsigned hold(unsigned delay) ;

  boost::none_t set(Error error) { error_ = error ; return boost::none ; }
    
} ; } /* Mcp3008 */ } /* Main */

#endif // _Main_Mcp3008_Host_h_
