// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Clock-Manager
//
// see BCM2835 ARM Peripherals: chapter 6.3: General Purpose GPIO Clocks
// see elinux.org/BCM2835_registers#CM
// see www.raspberrypi.org/forums/viewtopic.php?p=1187723#p1187723
// --------------------------------------------------------------------

#ifndef INCLUDE_Rpi_Cm_h
#define INCLDUE_Rpi_Cm_h

#include "Bus/Address.h"
#include "Peripheral.h"
#include <cassert>
#include <Neat/Bit/Word.h>
#include <Neat/Numerator.h>

namespace Rpi {

struct Cm 
{
    static constexpr auto PNo = Peripheral::PNo::make<0x101>() ;
  
    Cm(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address = Bus::Address::Base
	+ PNo.value() * Page::nbytes ;
    
    // ----------------------------------------------------------------
    // Aliases (there are more; though not tested yet)
    // ----------------------------------------------------------------

    enum class Alias : unsigned { Gp0=0,Gp1=1,Gp2=2,Pcm=3,Pwm=4,Slim=5,Uart=6 } ;

    using AliasN = Neat::Numerator<Alias,Alias::Uart> ;

    // ----------------------------------------------------------------
    // Wrapper for Peripheral Registers
    // ----------------------------------------------------------------

    template<uint32_t M> struct Register
    {
	static constexpr auto Mask = M ; 

	using Word = Neat::Bit::Word<uint32_t,Mask> ; 
	
	Word read() const { return Word::coset(*p) ; }
	
	void write(Word w) { (*p) = (0x5au << 24) | w.value() ; }

	Bus::Address const address() const { return address_ ; }

	Register(Bus::Address address,uint32_t volatile *p)
	    : address_(address),p(p) {}

    private:
	
	Bus::Address address_ ; uint32_t volatile *p ; 
    } ;

    // ----------------------------------------------------------------
    // Control (CTL) and Prescaler/Divider (DIV) Registers
    // ----------------------------------------------------------------

    struct Ctl : Register<0x7bf>
    {
	using Base = Register<Mask> ;
	Ctl(Base base) : Base(base) {}
	using Src    = Word::Set< 0,4> ;
	using Enab   = Word::Bit< 4> ;
	using Kill   = Word::Bit< 5> ;
	using Busy   = Word::Bit< 7> ;
	using Flip   = Word::Bit< 8> ;
	using Mash   = Word::Set< 9,2> ;
    } ;

    Ctl ctl(Alias alias)
    {
	uint32_t i ;
	switch (alias)
	{
	case Alias::Gp0:  i = 0x070 ; break ;
	case Alias::Gp1:  i = 0x078 ; break ;
	case Alias::Gp2:  i = 0x080 ; break ;
	case Alias::Pcm:  i = 0x098 ; break ;
	case Alias::Pwm:  i = 0x0a0 ; break ;
	case Alias::Slim: i = 0x0a8 ; break ;
	case Alias::Uart: i = 0x0f0 ; break ;
	default: assert(false) ; abort() ;
	}
	auto index = Page::Index::make(i/sizeof(uint32_t)) ;
	return Ctl::Base(Bus::Address(Address+i),&page->at(index)) ;
    }

    struct Div : Register<0xffffff>
    {
	using Base = Register<Mask> ;
	Div(Base base) : Base(base) {}
	using Fract  = Word::Set< 0,12> ; // "DIVF"
	using Intgr  = Word::Set<12,12> ; // "DIVI"
    } ;
    
    Div div(Alias alias)
    {
	uint32_t i ;
	switch (alias)
	{
	case Alias::Gp0:  i = 0x074 ; break ;
	case Alias::Gp1:  i = 0x07c ; break ;
	case Alias::Gp2:  i = 0x084 ; break ;
	case Alias::Pcm:  i = 0x09c ; break ;
	case Alias::Pwm:  i = 0x0a4 ; break ;
	case Alias::Slim: i = 0x0ac ; break ;
	case Alias::Uart: i = 0x0f4 ; break ;
	default: assert(false) ; abort() ;
	}
	auto index = Page::Index::make(i/sizeof(uint32_t)) ;
	return Div::Base(Bus::Address(Address+i),&page->at(index)) ;
    }
    
    struct Clock
    {
	Ctl ctl ; Div div ;
	Clock(Ctl ctl,Div div) : ctl(ctl),div(div) {}
    } ;
    
    Clock clock(Alias alias)
    {
	return Clock(ctl(alias),div(alias)) ;
    }
    
    // ----------------------------------------------------------------
    // Convenience
    // ----------------------------------------------------------------
    
    bool enabled(Alias i) { return ctl(i).read().test(Ctl::Enab::Digit) ; }
    bool    busy(Alias i) { return ctl(i).read().test(Ctl::Busy::Digit) ; }
    
    Ctl::Src  source(Alias i) { return Ctl::  Src(ctl(i).read()) ; }
    Ctl::Mash   mash(Alias i) { return Ctl:: Mash(ctl(i).read()) ; }
    Div::Intgr intgr(Alias i) { return Div::Intgr(div(i).read()) ; }
    Div::Fract fract(Alias i) { return Div::Fract(div(i).read()) ; }
    
    void enable(Alias i) ;
    
    void disable(Alias i) ;
    
    void kill(Alias i) ;
    
    void set(Alias      alias,
	     Ctl::Src     src,
	     Div::Intgr intgr,
	     Div::Fract fract,
	     Ctl::Mash   mash) ;

private:

    std::shared_ptr<Page> page ;
  
} ; }

#endif // INCLUDE_Rpi_Cm_h
