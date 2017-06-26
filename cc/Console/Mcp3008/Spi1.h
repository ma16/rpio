// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Mcp3008_Spi1_h_
#define _Console_Mcp3008_Spi1_h_

#include "Circuit.h"
#include <Neat/U32.h>
#include <Neat/uint.h>
#include <Rpi/Spi1.h>
#include <boost/optional.hpp>

namespace Console { namespace Mcp3008 {

struct Spi1
{
  using    Cs = Neat::uint<uint32_t,Neat::U32::Sig<Rpi::Spi1::   Cs>()> ;
  using Speed = Neat::uint<uint32_t,Neat::U32::Sig<Rpi::Spi1::Speed>()> ;
  
  static uint32_t makeTx(Circuit::Source source) ;

  static void setup(Rpi::Spi1*,Speed,Cs,bool full) ;
 
  // requires setup(full=false)
  static Circuit::Sample evalRx(uint32_t) ;
  
  enum : int
  {
    Success    =  0,
    WrongStart = -1, // sample must start with zero-bit
    Mismatch   = -2, // MSB sample doesn't match LSB sample
  } ;

  // requires setup(full=true)
  static int evalRx(uint32_t,Circuit::Sample*) ;
} ;

} } 

#endif // _Console_Mcp3008_Spi1_h_
