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

#ifndef _Rpi_Counter_h_
#define _Rpi_Counter_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: chapter 14: Free Running ARM Counter
// --------------------------------------------------------------------

#include "Page.h"
#include <Neat/cast.h>

namespace Rpi
{
  class Peripheral ;
  
  struct Counter
  {
    bool enabled() const
    {
      return 0 != (page->at<0x408/4>() & (1u<<9)) ;
    }
    
    void enable(bool on)
    {
      auto w = page->at<0x408/4>() ;
      uint32_t i = 1 ; i <<= 9 ;
      w &= ~i ; // := 0
      i = on ; i <<= 9 ;
      w |=  i ; // := {on}
      page->at<0x408/4>() = w ;
    }

    using Div = uint8_t ; 

    Div prescaler() const
    {
      return static_cast<Div>(page->at<0x408/4>() >> 16) ;
    }
    
    void prescale(Div div)
    {
      auto w = page->at<0x408/4>() ;
      uint32_t i = 0xff ; i <<= 16 ;
      w &= ~i ; // := 0
      i = div ; i <<= 16 ;
      w |=  i ; // := {div}
      page->at<0x408/4>() = w ;
    }
    
    uint32_t clock() const
    {
      return page->at<0x420/4>() ;
    }

  private:

    friend Peripheral ;
    
    std::shared_ptr<Page> page ; Counter(std::shared_ptr<Page> page) : page(page) {}
  } ;
}

#endif // _Rpi_Counter_h_
