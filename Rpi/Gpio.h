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

#ifndef _Rpi_Gpio_h_
#define _Rpi_Gpio_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "Page.h"
#include "Pin.h"
#include <Neat/Numerator.h>

namespace Rpi
{
  class  Peripheral ;

  struct Gpio 
  {
    // ----[ Mode ]----------------------------------------------------

    enum class Mode : unsigned char { In=0,Out=1,Alt5=2,Alt4=3,Alt0=4,Alt1=5,Alt2=6,Alt3=7 } ;

    using ModeN = Neat::Numerator<Mode,Mode::Alt3> ;
    
    Mode getMode(Pin pin) const ;
    
    void setMode(uint32_t set,Mode mode) ;
    void setMode(Pin      pin,Mode mode) ;

    // ----[ Level ]---------------------------------------------------
    
    uint32_t getLevels() const
    {
      return page->at<0x34/4>() ; /* GPLEV0 */
    } 

    bool getLevel(Pin pin) const
    {
      return 0 != ((1u<<pin.value()) & getLevels()) ;
    }

    // ----[ Event ]---------------------------------------------------

    enum class Event : unsigned char { Rise=0,Fall=1,High=2,Low=3,AsyncRise=4,AsyncFall=5 } ;
    
    using EventN = Neat::Numerator<Event,Event::AsyncFall> ;
    
    template<Event E> uint32_t enable(uint32_t set,bool on=true)
    {
      return rmw(page->at<(0x4c+EventN(E).n()*0xc)/4>(),set,on) ; 
      // 0x4c : GP-R -EN0 
      // 0x58 : GP-F -EN0 
      // 0x64 : GP-H -EN0
      // 0x70 : GP-L -EN0 
      // 0x7c : GP-AR-EN0 
      // 0x88 : GP-AF-EN0
    }

    uint32_t getEvents() const
    {
      return page->at<0x40/4>() ; /* GPEDS0 */
    }
    
    bool getEvent(Pin pin) const
    {
      return 0 != ((1u<<pin.value()) & getEvents()) ;
    }

    template<Event E> bool enable(Pin pin,bool on=true)
    {
      return 1& (enable<E>(1u<<pin.value(),on) >> pin.value()) ;
    }
    
    void enable(uint32_t set,bool on=false) ;
    // ...re/sets all event types: rise,fall,etc.
    
    void enable(Pin pin,bool on=false)
    {
      enable(1u<<pin.value(),on) ;
    }
      
    void reset(uint32_t set)
    {
      page->at<0x40/4>() = set ; /* GPEDS0 */
    }
    
    void reset(Pin pin)
    {
      reset(1u<<pin.value()) ;
    }

    // ----[ Pull ]----------------------------------------------------

    enum class Pull : unsigned char { Off=0,Down=1,Up=2 } ;

    using PullN = Neat::Numerator<Pull,Pull::Up> ;
    
    void setPull(uint32_t set,Pull how) ;
    void setPull(Pin      pin,Pull how) ;

    // ----[ Output ]--------------------------------------------------

    enum class Output : unsigned char { Hi=0,Lo=1 } ;

    using OutputN = Neat::Numerator<Output,Output::Lo> ;

    template<Output O> void setOutput(uint32_t set)
    {
      page->at<(0x1c+OutputN(O).n()*0xc)/4>() = set ; // 0x1c:GPSET0 ; 0x28:GPCLR0
    }
    
    template<Output O> void setOutput(Pin pin)
    {
      setOutput<O>(1u << pin.value()) ;
    }
    
    void setOutput(uint32_t set,Output o)
    {
      switch (o) {
      case Output::Hi: setOutput<Output::Hi>(set) ; break ;
      case Output::Lo: setOutput<Output::Lo>(set) ; break ;
      }
    }
    
    void setOutput(Pin pin,Output o)
    {
      setOutput(1u << pin.value(),o) ;
    }

  private:

    friend Peripheral ;
    
    std::shared_ptr<Page> page ; Gpio(std::shared_ptr<Page> page) : page(page) {}

    static uint32_t rmw(uint32_t volatile &r,uint32_t set,bool on) ;
  } ;
}

#endif // _Rpi_Gpio_h_
