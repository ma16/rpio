// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Cm_h_
#define _Rpi_Cm_h_

// --------------------------------------------------------------------
// Clock-Manager
//
// see BCM2835 ARM Peripherals: chapter 6.3: General Purpose GPIO Clocks
//
// [future] there are many more clock sources for internal use as
//   CM_SYS,CM_PERI,CM_PCM,CM_PWM,CM_UART,CM_PLL,CM_ARM, etc.
//   see elinux.org/BCM2835_registers#CM
// --------------------------------------------------------------------

#include "Peripheral.h"

namespace Rpi { struct Cm 
{
  Cm(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x101>())) {}
  
  // ----------------------------------------------------------------
  // There are three programmable clock-pulse generators (GPCLK0/1/2)
  //
  // Note that GPCLK1 on Pi-2 appears to be wired to the Ethernet
  // clock. Changing this clock may have unexpected results.
  // http://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks
  // ----------------------------------------------------------------

  using Index = Neat::Enum<unsigned,3> ;

  // ----------------------------------------------------------------
  // The three CP generators are each controlled by two registers:
  // Control and Divider.
  // ----------------------------------------------------------------
    
  static Page::Index ctl(Index i)
  {
    switch (i.value()) {
    case 0: return Page::Index::make<0x70u/4>() ; // CM_GP0CTL
    case 1: return Page::Index::make<0x78u/4>() ; // CM_GP1CTL
    case 2: return Page::Index::make<0x80u/4>() ; // CM_GP2CTL
    case 3: return Page::Index::make<0xa0u/4>() ; // CM_PWMCTL
    }
    abort() ;
  }
    
  static Page::Index div(Index i)
  {
    switch (i.value()) {
    case 0: return Page::Index::make<0x74u/4>() ; // CM_GP0DIV
    case 1: return Page::Index::make<0x7cu/4>() ; // CM_GP1DIV
    case 2: return Page::Index::make<0x84u/4>() ; // CM_GP2DIV
    case 3: return Page::Index::make<0xa4u/4>() ; // CM_PWMDIV
    }
    abort() ;
  }

  // ----------------------------------------------------------------
  // Control
  // ----------------------------------------------------------------
    
  struct Control
  {
    // A clock-pulse generator is driven by a {Source}:
    //                              Pi-0 |   Pi-2
    // ----------------------------------+--------------
    //   1 = ocillator       =    1.92+7 | 1.92+7
    //   5 = PLLC            =    1.00+9 | 1.20+9
    //   6 = PLLD            =    5.00+8 | 5.00+8
    //   7 = HDMI auxilliary =       0+0 | 2.16+8
    //
    // Note that the frequencies values shown above are actually _not_
    // defined in the specification but have been probed. So treat them
    // with caution. See also:
    // http://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks
      
    using Source = Neat::Enum<unsigned,16-1> ;
      
    // If the {Divider.Fract} value (see Divider below) is non-zero,
    // the generated clock-pulse shifts between:
    //   Mash=0: {Source}/({Intgr}-0) and {Source}/({Intgr}+0)
    //   Mash=1: {Source}/({Intgr}-0) and {Source}/({Intgr}+1)
    //   Mash=2: {Source}/({Intgr}-1) and {Source}/({Intgr}+2)
    //   Mash=3: {Source}/({Intgr}-3) and {Source}/({Intgr}+4)
      
    using Mash = Neat::Enum<unsigned,4-1> ;
      
    bool   enable ;  // r/w
    Source source ;  // r/w
    Mash     mash ;  // r/w
    bool     kill ;  // r/w
    bool     busy ;  // read-only

    // notes:
    // --the cp generator might still be busy (for a while) after enable=false
    // --sometimes the clock-pulse gets stuck; use kill to revive
    // --there is also flip bit, though it is not covered here (always 0)
      
    uint32_t serialize() const
    {
      uint32_t w = 0 ;
      w |= 0x5a ;         w <<= 15 ; // 31 - 24 (password)
      w |= mash.value() ; w <<=  4 ; // 10 -  9 [todo] only bit 9 for CM_GP2CTL? (http://elinux.org/BCM2835_registers#CM)
      w |= kill ;         w <<=  1 ; //  5 -  5
      w |= enable ;       w <<=  4 ; //  4 -  4
      w |= source.value() ;          //  3 -  0
      return w ;
    }
      
    static Control deserialize(uint32_t w)
    {
      auto source = w & 0xf ; w >>= 4 ; //  0 -  3
      auto enable = w & 0x1 ; w >>= 1 ; //  4 -  4
      auto   kill = w & 0x1 ; w >>= 2 ; //  5 -  5
      auto   busy = w & 0x1 ; w >>= 2 ; //  7 -  7
      auto   mash = w & 0x3 ;           //  9 - 10
      return Control(enable,Source::make(source),Mash::make(mash),kill,busy) ;
    }
      
    Control(bool enable,Source source,Mash mash,bool kill,bool busy=false)
      : enable(enable),source(source),mash(mash),kill(kill),busy(busy) {}
  } ;

  Control getControl(Index i) const { return Control::deserialize(page->at(ctl(i))) ; }

  void set(Index i,Control const &control) { page->at(ctl(i)) = control.serialize() ; }
    
  // ----------------------------------------------------------------
  // Divider
  // ----------------------------------------------------------------
    
  struct Divider
  {
    // The {Source} frequency is lowered by a {Divider}. This divider
    // contains an integer part {Intgr} and a fractional part {Fract}.
      
    using Intgr = Neat::Enum<unsigned,4096-1> ; Intgr intgr ;
    using Fract = Neat::Enum<unsigned,4096-1> ; Fract fract ;

    // [note] according to 
    //   http://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks
    // {Intgr}: 2-4095. 
    // {Fract}: 0-4095.

    // [note] sometimes (noticed on Pi-2 with source=1,mash=0), when
    // changing the divider, the clock-pulse drops to ~4.69 kHz w/o
    // considering the setting. The only solution appears to kill the
    // clock.
      
    uint32_t serialize() const
    {
      uint32_t w = 0 ;
      w |= 0x5a ;          w <<= 12 ; // 31 - 24 (password)
      w |= intgr.value() ; w <<= 12 ; // 23 - 12
      w |= fract.value() ;            // 11 -  0
      return w ;
    }
      
    static Divider deserialize(uint32_t w)
    {
      auto fract = w & 0xfff ; w >>= 12 ; //  0 - 11
      auto intgr = w & 0xfff ;            // 12 - 23
      return Divider(Intgr::make(intgr),Fract::make(fract)) ;
    }
      
    Divider(Intgr intgr,Fract fract) : intgr(intgr),fract(fract) {}
  } ;

  Divider getDivider(Index i) const { return Divider::deserialize(page->at(div(i))) ; }

  void set(Index i,Divider const &divider) { page->at(div(i)) = divider.serialize() ; }
    
  // ----------------------------------------------------------------
  // Convenience
  // ----------------------------------------------------------------
    
  bool           enabled(Index i) const { return getControl(i).enable ; }
  bool              busy(Index i) const { return getControl(i).  busy ; }
  Control::Source source(Index i) const { return getControl(i).source ; }
  Control::Mash     mash(Index i) const { return getControl(i).  mash ; }
  Divider::Intgr   intgr(Index i) const { return getDivider(i). intgr ; }
  Divider::Fract   fract(Index i) const { return getDivider(i). fract ; }
    
  void enable(Index i) ;
    
  void disable(Index i) ;
    
  void kill(Index i) ;
    
  void set(Index i,Control::Source source,Divider::Intgr intgr,Divider::Fract fract,Control::Mash mash) ;

private:

  std::shared_ptr<Page> page ;
  
} ; }

#endif // _Rpi_Cm_h_
