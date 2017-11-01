// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Peripheral_h
#define INCLUDE_Rpi_Peripheral_h

// The BCM2835 periperal address range covers 18 documented memory
// pages within an (partially used) address range of 0x1000,000
// bytes (0x1000 pages). See BCM2835 ARM Peripherals (chapter §§):
//
//   §§  page   used  peripheral
// -----------------------------------
// 12.1 003:(000-01b) VC System Timer
//  4.2 007:(000-fff) VC DMA (channel 0-14 + control)
//  7.5 00b:(200-227) ARM Interrupts
// 14.2 00b:(400-423) ARM Timer
//    - 00b:(880-8bf) ARM Mailbox [elinux.org/RPi_Framebuffer]
//    - 100:(02c-037) VC GPIO Pads Control [www.scribd.com/doc/101830961]
//    - 101:(000-06f) VC CM Clock Pulse [elinux.org/BCM2835_registers#CM]
//  6.3 101:(070-087) VC CM_GP (GPIO) Clock Pulse
//    - 101:(088-1c3) VC CM Clock Pulse [elinux.org/BCM2835_registers#CM]
//  6.1 200:(000-0b3) VC GPIO
// 13.4 201:(000-08f) VC UART
//  8.8 203:(000-023) VC PCM
// 10.5 204:(000-017) VC SPI
//  3.2 205:(000-01f) VC Serial (BSC0)
//  9.6 20c:(000-027) VC PWM [elinux.org/BCM2835_datasheet_errata#p125]
// 11.2 214:(000-03f) VC SPI (Slave)
//  2.1 215:(000-0d7) VC Auxiliary (UART1,SPI1,SPI2)
//  5   300:(000-0ff) VC EMMC
//  3.2 804:(000-01f) VC Serial (BSC1)
//  3.2 805:(000-01f) VC Serial (BSC2)
// 15.2 980:(080-08b) VC USB (Synopsys IP)
//  4.2 e05:(000-0ff) VC DMA (channel 15)
// ----------------------
//      total: 18 documented pages
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
#include "Register.h"

#include <Neat/Bit/Word.h>
#include <Neat/Enum.h>
#include <Posix/Fd.h> // uoff_t

#include <map>

namespace Rpi { struct Peripheral
{
    static std::shared_ptr<Peripheral> make(Posix::Fd::uoff_t addr) ;
    // ...shared_ptr so it can easily be passed around incl. ownership

    // figure out the physical ARM address automatically...
    static Posix::Fd::uoff_t by_devtree() ;
    static Posix::Fd::uoff_t by_cpuinfo() ;
    
    using PNo = Neat::Enum<unsigned,0x1000-1> ;
    
    std::shared_ptr<Page>       page(PNo no) ;
    std::shared_ptr<Page const> page(PNo no) const ;
    // ...shared_ptr so it can easily be passed around incl. ownership

    uint32_t volatile const & at(size_t i) const ;
    uint32_t volatile       & at(size_t i)       ;
    // ...i is relative to base_page as a byte-offset
    //    (which must be word-aligned)

    uint32_t base_addr() const { return base_page * Page::nbytes ; }
  
    Peripheral           (Peripheral const&) = delete ;
    Peripheral& operator=(Peripheral const&) = delete ;

    template<typename Traits> Register::Pointer<Traits> at()
    {
	return & page(Traits::PageNo)->at(Traits::Index) ;
	// ...that's a fairly expensive call (compared with base.at())
    }

    template<uint32_t P> Register::Base<P> page()
    {
	return & page(PNo::make<P>())->at<0>() ;
    }

private:

    using Map = std::map<unsigned,std::shared_ptr<Page>> ;
      
    size_t base_page ;

    Posix::Fd::shared_ptr mem ; // file descriptor for "/dev/mem"
  
    mutable Map map ;

    Peripheral(size_t base_page,Posix::Fd::shared_ptr mem,Map &&map)
	: base_page(base_page)
	, mem            (mem)
	, map (std::move(map))
	{ }
} ; }

#endif // INCLUDE_Rpi_Peripheral_h
