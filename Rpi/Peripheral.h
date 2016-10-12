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

#ifndef _Rpi_Peripheral_h_
#define _Rpi_Peripheral_h_

// The BCM2835 periperal address range covers 18 documented memory
// pages within an (partially used) address range of 0x1000,000
// bytes (0x1000 pages). See BCM2835 ARM Peripherals (chapter §§):
//
//   §§  page:offset      peripheral
// -----------------------------------
// 12.1 (003:000,003:01c] VC System Timer
//  4.2 (007:000,008:000] VC DMA (channel 0-14 + control)
//  7.5 (00b:200,00B:228] ARM Interrupts
// 14.2 (00b:400,00B:424] ARM Timer
//    - (00b:880,00b:8c0] ARM Mailbox [http://elinux.org/RPi_Framebuffer]
//    - (100:02c,100:038] VC GPIO Pads Control [http://www.scribd.com/doc/101830961]
//  6.3 (101:070,101:088] VC Clock Pulse
//  6.1 (200:000,200:0b4] VC GPIO
// 13.4 (201:000,201:090] VC UART
//  8.8 (203:000,203:024] VC PCM
// 10.5 (204:000,204:018] VC SPI
//  3.2 (205:000,205:020] VC Serial (BSC0)
//  9.6 (20c:000,20c:028] VC PWM [http://elinux.org/BCM2835_datasheet_errata#p125]
// 11.2 (214:000,214:040] VC SPI (Slave)
//  2.1 (215:000,215:0d8] VC Auxiliary (UART1,SPI1,SPI2)
//  5   (300:000,300:100] VC EMMC
//  3.2 (804:000,804:020] VC Serial (BSC1)
//  3.2 (805:000,805:020] VC Serial (BSC2)
// 15.2 (980:080,980:08c] VC USB (Synopsys IP)
//  4.2 (e05:000,e05:100] VC DMA (channel 15)
// ----------------------
//      total: 18 pages
//
// For the Video Core, the BCM2835 uses a coarse-grained MMU to map "ARM
// physical addresses" onto "system bus addresses". System bus addresses
// start at 0x7e00,0000.
//
// The BCM2835 peripherals start at ARM physical address 0x2000,0000. 
//
// The BCM2836 peripherals start at ARM physical address 0x3f00,0000. 
// (so the upper 0x100,0000 bytes _RAM_ are not addressable by the ARM core?)

#include "Error.h"
#include "Page.h"

#include <Neat/Enum.h>
#include <Posix/Fd.h> // uoff_t

#include <map>

namespace Rpi
{
  struct Peripheral
  {
    static std::shared_ptr<Peripheral> make(Posix::Fd::uoff_t addr) ;
    // ...shared_ptr so it can easily be passed around incl. ownership

    // bus address 0x7e00,0000 is available at physical ARM address...
    static Posix::Fd::uoff_t for_bcm2835() { return Posix::Fd::uoff_t::init<0x20000000u>() ; }
    static Posix::Fd::uoff_t for_bcm2836() { return Posix::Fd::uoff_t::init<0x3f000000u>() ; }
    static Posix::Fd::uoff_t for_bcm2837() { return Posix::Fd::uoff_t::init<0x3f000000u>() ; }

    // figure out the physical ARM address automatically...
    static Posix::Fd::uoff_t by_devtree() ;
    static Posix::Fd::uoff_t by_cpuinfo() ;
    
    using PNo = Neat::Enum<unsigned,0x1000-1> ;
    
    std::shared_ptr<Page> page(PNo no) ;
    // ...shared_ptr so it can easily be passed around incl. ownership

    Peripheral           (Peripheral const&) = delete ;
    Peripheral& operator=(Peripheral const&) = delete ;
    
  private:

    using Map = std::map<unsigned,std::shared_ptr<Page>> ;
      
    size_t base_page ; Map map ;
    
    Peripheral(size_t base_page_,Map &&map_) : base_page(base_page_),map(std::move(map_)) {}
  } ;
}

#endif // _Rpi_Peripheral_h_
