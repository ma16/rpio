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
//
// Please refer to the MAX7219 data sheet.

#include "Host.h"
#include <chrono>

void Main::Max7219::Host::send(uint16_t data)
{
  // the level (high/low) on DAT={datPin} is shiftet into the MAX7219
  // with each LH-edge on CLK={clkPin}. {data} is transmitted most-
  // signifcant-bit (msb) first within 16 clock-pulses.
  for (unsigned i=0 ; i<16 ; ++i) {
    this->gpio->setOutput<Rpi::Gpio::Output::Lo>(this->clkPin) ;
    if (0 == (data & 0x8000u)) this->gpio->setOutput<Rpi::Gpio::Output::Lo>(this->datPin) ;
    else                       this->gpio->setOutput<Rpi::Gpio::Output::Hi>(this->datPin) ;
    this->hold() ;
    this->gpio->setOutput<Rpi::Gpio::Output::Hi>(this->clkPin) ; 
    this->hold() ;
    data = static_cast<decltype(data)>(data << 1) ;
  }
  // {clkPin} is high on return
}

void Main::Max7219::Host::latch()
{
  // the contents of the MAX7219's shift-register is latched-in (i.e.
  // loaded by the MAX7219) with an LH-edge on LOAD={loadPin}.
  // --for a single MAX7219: a latch() call should be executed after
  //   each send() call.
  // --for <n> daisy-chained MAX7219: a latch() call should be executed
  //   after <n> send() calls -- however, there may be situations when
  //   it is appropriate to latch() after a single send().
  this->gpio->setOutput<Rpi::Gpio::Output::Lo>(this->loadPin) ; this->hold() ;
  this->gpio->setOutput<Rpi::Gpio::Output::Hi>(this->loadPin) ; this->hold() ;
  // {loadPin} is high on return
}
    
Main::Max7219::Host::Host(Rpi::Gpio *gpio,Rpi::Pin loadPin,Rpi::Pin clkPin,Rpi::Pin datPin)
  : gpio(gpio),loadPin(loadPin),clkPin(clkPin),datPin(datPin)
{
  gpio->setMode(loadPin,Rpi::Gpio::Mode::Out) ;
  gpio->setMode( clkPin,Rpi::Gpio::Mode::Out) ;
  gpio->setMode( datPin,Rpi::Gpio::Mode::Out) ;
  // [future] recall previous states and recover them in d'tor
}

void Main::Max7219::Host::hold()
{
  auto t0 = std::chrono::steady_clock::now() ;
  decltype(t0) ti ;
  do ti = std::chrono::steady_clock::now() ;
  while (std::chrono::duration<double>(ti-t0).count() < 5E-8) ;
  // the MAX7219 maximum signal rate is 10 MHz. this would make 5E-8
  // seconds for high/low level each -- on a 1:1 duty cycle. however,
  // suspending the thread for such a little time appears not to be
  // possible. for example, nanosleep() seems to suspend the thread
  // significantly longer. though, if throughput doesn't matter, it
  // won't hurt either.
  //
  // if throughput matters, we have to check the passed time in a
  // busy loop. still, requesting a point in time alone may take
  // longer than 5E-8, and we need to request it at least twice. for
  // example, clock_gettime() -- which is used by std::chrono --
  // appears to take more than 1us. -- there are ARM and VC timers
  // which might be more suitable.
  //
  // [future] use counter-loop if throughput is really an issue. since
  // the execution time of a counter-loop is highly system dependent,
  // this requires at least a runtime test beforhand.
}
