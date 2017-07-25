// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Clock-Manager
//
// see BCM2835 ARM Peripherals: chapter 6.3: General Purpose GPIO Clocks
// see elinux.org/BCM2835_registers#CM
// see www.raspberrypi.org/forums/viewtopic.php?p=1187723#p1187723
// --------------------------------------------------------------------

#ifndef _Rpi_Cm_h_
#define _Rpi_Cm_h_

#include "Peripheral.h"
#include <cassert>
#include <Neat/Numerator.h>

namespace Rpi {

struct Cm 
{
    Cm(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x101>())) {}
  
    // Each clock-generator is set-up by control and divider register.
    
    // ----------------------------------------------------------------
    // Control Register
    // ----------------------------------------------------------------
    
    struct Control
    {
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
	// --the cp generator might still be busy (for a while) after
	//   enable=false
	// --sometimes the clock-pulse gets stuck; use kill to revive
	// --there is also flip bit, though it is not covered here
	//   (always 0)
      
	uint32_t serialize() const
	{
	    uint32_t w = 0 ;
	    w |= 0x5a ;         w <<= 15 ; // 31 - 24 (password)
	    w |= mash.value() ; w <<=  4 ; // 10 -  9 [todo] only bit 9 for
	    //      ... CM_GP2CTL? (http://elinux.org/BCM2835_registers#CM)
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
	    return Control(enable,Source::make(source),
			   Mash::make(mash),kill,busy) ;
	}
      
	Control(bool enable,Source source,Mash mash,bool kill,bool busy=false)
	    : enable(enable),source(source),mash(mash),kill(kill),busy(busy) {}
    } ;

    // ----------------------------------------------------------------
    // Divider Register
    // ----------------------------------------------------------------
    
    struct Divider
    {
	// The {Source} frequency is lowered by a {Divider}. This divider
	// contains an integer part {Intgr} and a fractional part {Fract}.
      
	using Intgr = Neat::Enum<unsigned,4096-1> ; Intgr intgr ;
	using Fract = Neat::Enum<unsigned,4096-1> ; Fract fract ;

	// [note] according to 
	// http://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks
	// {Intgr}: 2-4095. 
	// {Fract}: 0-4095.

	// [note] sometimes (noticed on Pi-2 with source=1,mash=0),
	// when changing the divider, the clock-pulse drops to ~4.69
	// kHz w/o acception the new setting. The only solution appears
	// to kill the clock generator.
      
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

    // ----------------------------------------------------------------
    // Aliases (there are more; though not tested yet)
    // ----------------------------------------------------------------

    enum class Alias : unsigned { Gp0=0,Gp1=1,Gp2=2,Pwm=3 } ;

    using AliasN = Neat::Numerator<Alias,Alias::Pwm> ;

    static Page::Index ctl(Alias alias)
    {
	switch (alias)
	{
	case Alias::Gp0: return Page::Index::make<0x70/4>() ;
	case Alias::Gp1: return Page::Index::make<0x78/4>() ;
	case Alias::Gp2: return Page::Index::make<0x80/4>() ;
	case Alias::Pwm: return Page::Index::make<0xa0/4>() ;
	default:assert(false) ;
	}
	abort() ;
    }
    
    static Page::Index div(Alias alias)
    {
	switch (alias)
	{
	case Alias::Gp0: return Page::Index::make<0x74/4>() ;
	case Alias::Gp1: return Page::Index::make<0x7c/4>() ;
	case Alias::Gp2: return Page::Index::make<0x84/4>() ;
	case Alias::Pwm: return Page::Index::make<0xa4/4>() ;
	default:assert(false) ;
	}
	abort() ;
    }
    
    Control getControl(Alias i) const
    {
	return Control::deserialize(page->at(ctl(i))) ;
    }

    void set(Alias i,Control const &control)
    {
	page->at(ctl(i)) = control.serialize() ;
    }
    
    Divider getDivider(Alias i) const
    {
	return Divider::deserialize(page->at(div(i))) ;
    }

    void set(Alias i,Divider const &divider)
    {
	page->at(div(i)) = divider.serialize() ;
    }
    
    // ----------------------------------------------------------------
    // Convenience
    // ----------------------------------------------------------------
    
    bool           enabled(Alias i) const { return getControl(i).enable ; }
    bool              busy(Alias i) const { return getControl(i).  busy ; }
    Control::Source source(Alias i) const { return getControl(i).source ; }
    Control::Mash     mash(Alias i) const { return getControl(i).  mash ; }
    Divider::Intgr   intgr(Alias i) const { return getDivider(i). intgr ; }
    Divider::Fract   fract(Alias i) const { return getDivider(i). fract ; }
    
    void enable(Alias i) ;
    
    void disable(Alias i) ;
    
    void kill(Alias i) ;
    
    void set(Alias                i,
	     Control::Source source,
	     Divider::Intgr   intgr,
	     Divider::Fract   fract,
	     Control::Mash     mash) ;

private:

    std::shared_ptr<Page> page ;
  
} ; }

#endif // _Rpi_Cm_h_
