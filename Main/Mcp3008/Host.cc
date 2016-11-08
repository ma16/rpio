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

#include "Host.h"
#include <chrono>

static auto const Lo = Rpi::Gpio::Output::Lo ;
static auto const Hi = Rpi::Gpio::Output::Hi ;

Main::Mcp3008::Host::Host(Rpi::Peripheral *rpi,
			  Rpi::Pin   csPin,
			  Rpi::Pin  clkPin,
			  Rpi::Pin   toPin,
			  Rpi::Pin fromPin,
			  Timing const &t,
			  bool monitor) 
  : rpi(rpi),csPin(csPin),clkPin(clkPin),toPin(toPin),fromPin(fromPin),t(t),monitor(monitor)
{
  rpi->gpio()->setMode(this->  csPin,Rpi::Gpio::Mode::Out) ;
  rpi->gpio()->setMode(this-> clkPin,Rpi::Gpio::Mode::Out) ;
  rpi->gpio()->setMode(this->  toPin,Rpi::Gpio::Mode::Out) ;
  rpi->gpio()->setMode(this->fromPin,Rpi::Gpio::Mode:: In) ;
  
  this->rpi->gpio()->setOutput<Hi>(this-> csPin) ; 
  this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
  this->rpi->gpio()->setOutput<Lo>(this-> toPin) ; 

  // [future] recall previous states and recover them in d'tor

  // make sure the ARM counter is on and runs at 100 Mhz
  if (!this->rpi->counter()->enabled())
    throw std::runtime_error("please enable ARM counter") ;
  if (this->rpi->counter()->prescaler() != 3)
    throw std::runtime_error("please set ARM prescaler to <3>") ;
}

boost::optional<Main::Mcp3008::Host::Sample> Main::Mcp3008::Host::query(Source source) 
{
  // *** Reset ***
  this->hold(this->t.csh) ; 
  // ...[future] use (fast wrap around) ARM counter only within this
  // function; another (not that fast) counter between function calls.

  if (this->monitor) {
    if (this->rpi->gpio()->getLevel(this->  csPin) !=  true) return this->set(Error::StartCSHi) ;
    if (this->rpi->gpio()->getLevel(this-> clkPin) != false) return this->set(Error::StartClkLo) ;
    if (this->rpi->gpio()->getLevel(this->  toPin) != false) return this->set(Error::StartDInLo) ;
    if (this->rpi->gpio()->getLevel(this->fromPin) !=  true) return this->set(Error::StartDOutHi) ;
    // ...[future] read levels with a single command (speedup)
  }
  
  // *** Enable device ***
  this->rpi->gpio()->setOutput<Lo>(this->csPin) ; // CS enable
  this->rpi->gpio()->setOutput<Hi>(this->toPin) ; // start bit
  auto t0 = this->hold(this->t.sucs) ; 
  this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; 
  // "an analog sample is acquired for the next 1.5 clocks"

  // *** Send Source ***
  auto data = source.value() ;
  for (auto i=0 ; i<4 ; ++i) {
    // MSB first; DIN gets latched with CLK=LH
    this->hold(this->t.hd) ;
    this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
    if (0 == (data & 0x8)) this->rpi->gpio()->setOutput<Lo>(this->toPin) ;
    else                   this->rpi->gpio()->setOutput<Hi>(this->toPin) ;
    // ...[future] can be skipped if no change (speedup)
    data <<= 1 ;
    this->hold(this->t.su) ; 
    this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; 
  }

  // *** Start Sampling ***
  // "The device will begin to sample the analog input on the fourth
  // rising edge of the clock after the start bit has been received.
  // The sample period will end on the falling edge of the fifth clock
  // following the start bit." 
  this->hold(this->t.hd) ; // DIN=D0
  this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
  this->rpi->gpio()->setOutput<Lo>(this->toPin) ;
  // ...we use  a 'defined' level even though "don't care"
  this->hold(this->t.su) ; 
  this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; // DIN=don't care
  this->hold(this->t.hd) ; 
  this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 

  // *** Output gets Enabled *** (DOUT=Lo)
  // "Once the D0 bit is input, one more clock is required to complete
  // the sample and hold period (DIN is a “don’t care” for this clock).
  // On the falling edge of the next clock, the device will output a
  // low null bit." 
  this->hold(this->t.en) ;
  if (this->monitor) {
    if (this->rpi->gpio()->getLevel(this->fromPin) != false)
      return this->set(Error::RespDOutLo) ;
  }
  
  // *** Receive Sample ***
  // "The next 10 clocks will output the result of the conversion
  // with MSB first. Data is always output from the device on the
  // falling edge of the clock."
  this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; 
  this->hold(this->t.hi) ;
  this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
  this->hold(this->t.do_) ; 
  auto sample = static_cast<Sample::Domain>(this->rpi->gpio()->getLevel(this->fromPin)) ;
  for (auto i=1 ; i<10 ; ++i) {
    this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; 
    this->hold(this->t.hi) ; 
    this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
    this->hold(this->t.lo) ; 
    sample <<= 1 ;
    sample |= this->rpi->gpio()->getLevel(this->fromPin) ;
  }

  if (this->monitor) {
    // *** Verify Transmittion ***
    // "After completing the data transfer, if further clocks are applied
    // with CS low, the A/D converter will output LSB first data, then
    // followed with zeros indefinitely."
    auto check = sample ;
    for (auto i=1 ; i<10 ; ++i) {
      this->rpi->gpio()->setOutput<Hi>(this->clkPin) ; 
      this->hold(this->t.hi) ; 
      this->rpi->gpio()->setOutput<Lo>(this->clkPin) ; 
      this->hold(this->t.lo) ;
      check >>= 1 ;
      if (static_cast<bool>(check & 1) != this->rpi->gpio()->getLevel(this->fromPin))
	return this->set(Error::RecvError) ;
    }
  }
  
  // [todo] what about:
  // -- t-R (D OUT Rise Time)
  // -- t-F (D OUT Fall Time)

  // *** Disable Device ***/
  this->rpi->gpio()->setOutput<Hi>(this->csPin) ;
  auto t1 = this->hold(this->t.dis) ;
  if (this->monitor) {
    if (t1-t0 > this->t.bled/10)
      return this->set(Error::BledTimeout) ;
    if (this->rpi->gpio()->getLevel(this->fromPin) != true)
      return this->set(Error::EndDOutHi) ;
  }

  return Sample::make(sample) ;
}

unsigned Main::Mcp3008::Host::hold(unsigned delay)
{
  auto t0 = this->rpi->counter()->clock() ;
  // [note] on a slow clock (which may return the same value several
  // times) we'd need to loop to the next point in time (i.e. VC Timer).
  auto t1 = this->rpi->counter()->clock() ;
  while (t1-t0 < delay/10) 
    t1 = this->rpi->counter()->clock() ;
  return t1 ;
}
