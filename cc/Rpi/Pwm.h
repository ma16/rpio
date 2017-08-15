// BSD 2-Clause License, see github.com/ma16/rpio

// see Pwm.md for details

#ifndef INCLUDE_Rpi_Pwm_h
#define INCLUDE_Rpi_Pwm_h

#include "Peripheral.h"
#include "Register.h"
#include <Neat/Enum.h>

namespace Rpi {

struct Pwm 
{
    static constexpr auto PNo = Peripheral::PNo::make<0x20c>() ;
    
    Pwm(Peripheral *p) : page(p->page(PNo)) {}

    static constexpr auto Address = Bus::Address::Base + PNo.value() * Page::nbytes ;

    struct Control : Register::Masked<Address+0x000,0xbfff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Control(Base base) : Base(base) {}
	using Pwen1 = Word::Bit< 0> ;
	using Mode1 = Word::Bit< 1> ; 
	using Rptl1 = Word::Bit< 2> ; 
	using Sbit1 = Word::Bit< 3> ; 
	using Pola1 = Word::Bit< 4> ; 
	using Usef1 = Word::Bit< 5> ; 
	using Clrf  = Word::Bit< 6> ; 
	using Msen1 = Word::Bit< 7> ; 
	using Pwen2 = Word::Bit< 8> ; 
	using Mode2 = Word::Bit< 9> ; 
	using Rptl2 = Word::Bit<10> ; 
	using Sbit2 = Word::Bit<11> ; 	
	using Pola2 = Word::Bit<12> ; 
	using Usef2 = Word::Bit<13> ; 
	using Msen2 = Word::Bit<15> ;
    } ;

    struct Status : Register::Masked<Address+0x004,0x73f>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Status(Base base) : Base(base) {}
	using Full = Word::Bit< 0> ; 
	using Empt = Word::Bit< 1> ; 
	using Werr = Word::Bit< 2> ; 
	using Rerr = Word::Bit< 3> ; 
	using Gap1 = Word::Bit< 4> ; 
	using Gap2 = Word::Bit< 5> ; 
	using Berr = Word::Bit< 8> ; 
	using Sta1 = Word::Bit< 9> ; 
	using Sta2 = Word::Bit<10> ; 
    } ;

    struct DmaC : Register::Masked<Address+0x008,0x8000ffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	DmaC(Base base) : Base(base) {}
	using Dreq   = Word::Set<0,8> ;
	using Panic  = Word::Set<8,8> ;
	using Enable = Word::Set<31,1> ;
    } ;

    using Range1 = Register::Word<Address+0x010> ;
    using  Data1 = Register::Word<Address+0x014> ;
    using   Fifo = Register::Word<Address+0x018> ;
    using Range2 = Register::Word<Address+0x020> ;
    using  Data2 = Register::Word<Address+0x024> ;

    Control control() { return Control(&page->at<Control::Offset/4>()) ; }
    Data1     data1() { return   Data1(&page->at<  Data1::Offset/4>()) ; }
    Data2     data2() { return   Data2(&page->at<  Data2::Offset/4>()) ; }
    DmaC       dmaC() { return    DmaC(&page->at<   DmaC::Offset/4>()) ; }
    Fifo       fifo() { return    Fifo(&page->at<   Fifo::Offset/4>()) ; }
    Range1   range1() { return  Range1(&page->at< Range1::Offset/4>()) ; }
    Range2   range2() { return  Range2(&page->at< Range2::Offset/4>()) ; }
    Status   status() { return  Status(&page->at< Status::Offset/4>()) ; }
    
    struct Bank
    {
	Control::Word::Digit mode ;
	Control::Word::Digit msen ;
	Control::Word::Digit pola ;
	Control::Word::Digit pwen ;
	Control::Word::Digit rptl ;
	Control::Word::Digit sbit ;
	Control::Word::Digit usef ;
	Status::Word::Digit gap ;
	Status::Word::Digit sta ;
    } ;

    using Index = Neat::Enum<unsigned,1> ; 

    static constexpr auto Channel1 = Index::make<0>() ;
    static constexpr auto Channel2 = Index::make<1>() ;

    static constexpr Bank bank[2] =
    {
	{
	    Control::Mode1::Digit,
	    Control::Msen1::Digit,
	    Control::Pola1::Digit,
	    Control::Pwen1::Digit,
	    Control::Rptl1::Digit,
	    Control::Sbit1::Digit,
	    Control::Usef1::Digit,
	    Status::Gap1::Digit,
	    Status::Sta1::Digit,
	},
	{
	    Control::Mode2::Digit,
	    Control::Msen2::Digit,
	    Control::Pola2::Digit,
	    Control::Pwen2::Digit,
	    Control::Rptl2::Digit,
	    Control::Sbit2::Digit,
	    Control::Usef2::Digit,
	    Status::Gap2::Digit,
	    Status::Sta2::Digit,
	}
    } ;

    static constexpr Bank select(Index i) { return bank[i.value()] ; }

private:
    
    std::shared_ptr<Page> page ; 

} ; }

#endif // INCLUDE_Rpi_Pwm_h
