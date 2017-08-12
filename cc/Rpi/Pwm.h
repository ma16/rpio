// BSD 2-Clause License, see github.com/ma16/rpio

// see Pwm.md for details

#ifndef INCLUDE_Rpi_Pwm_h
#define INCLUDE_Rpi_Pwm_h

#include "Bus/Address.h"
#include "Dma/Ti.h" // Permap for DMA pacing
#include "Peripheral.h"
#include <Neat/Bit/Word.h>
#include <Neat/Enum.h>

namespace Rpi {

struct Pwm 
{
    using Index = Neat::Enum<unsigned,1> ; 

    static constexpr auto Channel1 = Index::make<0>() ;
    static constexpr auto Channel2 = Index::make<1>() ;
    
    struct Control
    {
	static constexpr uint32_t Mask = 0xbfffu ;

	using Word = Neat::Bit::Word<uint32_t,Mask> ; 

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
	
	struct Bank
	{
	    Word::Digit mode ;
	    Word::Digit msen ;
	    Word::Digit pola ;
	    Word::Digit pwen ;
	    Word::Digit rptl ;
	    Word::Digit sbit ;
	    Word::Digit usef ;
	    static Bank const& select(Index) ;
	} ;
	
	Word read() const { return Word::coset(*p) ; }
	
	void write(Word w) { (*p) = w.value() ; }

	static constexpr auto Address = Bus::Address(0x7e20c000) ;
	
    private:
	
	friend Pwm ; uint32_t volatile *p ;
	
	Control(uint32_t volatile *p) : p(p) {}
    } ;
	    
    Control control() { return & page->at<0x0/4>() ; }
    
    struct Status 
    {
	static constexpr auto Mask = 0x73f ;
	
	using Word = Neat::Bit::Word<uint32_t,Mask> ;

	using Full = Word::Bit< 0> ; 
	using Empt = Word::Bit< 1> ; 
	using Werr = Word::Bit< 2> ; 
	using Rerr = Word::Bit< 3> ; 
	using Gap1 = Word::Bit< 4> ; 
	using Gap2 = Word::Bit< 5> ; 
	using Berr = Word::Bit< 8> ; 
	using Sta1 = Word::Bit< 9> ; 
	using Sta2 = Word::Bit<10> ; 

	struct Bank
	{
	    Word::Digit gap ;
	    Word::Digit sta ;
	    static Bank const& select(Index) ;
	} ;
	
	Word read() const { return Word::coset(*p) ; }

	void clear(Word w) { (*p) = w.value() ; }

	static constexpr auto Address = Bus::Address(0x7e20c004) ;
	
    private:
	
	friend Pwm ; uint32_t volatile *p ;

	Status(uint32_t volatile *p) : p(p) {}
    } ;

    Status status() { return & page->at<0x4/4>() ; }

    struct Fifo
    {
	void write(uint32_t w) { (*p) = w ; }

	static constexpr auto Address = Bus::Address(0x7e20c018) ;
	
    private:
	
	friend Pwm ; uint32_t volatile *p ;

	Fifo(uint32_t volatile *p) : p(p) {}
    } ;

    Fifo fifo() { return & page->at<0x18/4>() ; }

    struct Range
    {
	uint32_t read() const { return (*p) ; }

	void write(uint32_t w) { (*p) = w ; }

    private:
	
	friend Pwm ; uint32_t volatile *p ;

	Range(uint32_t volatile *p) : p(p) {}
    } ;

    Range range(Index i)
    {
	return (i.value() == 0)
	    ? & this->page->at<0x10/4>() 
	    : & this->page->at<0x20/4>() ;
    }
  
    struct Data
    {
	uint32_t read() const { return (*p) ; }

	void write(uint32_t w) { (*p) = w ; }

    private:
	
	friend Pwm ; uint32_t volatile *p ;

	Data(uint32_t volatile *p) : p(p) {}
    } ;

    Data data(Index i)
    {
	return (i.value() == 0)
	    ? & this->page->at<0x14/4>() 
	    : & this->page->at<0x24/4>() ;
    }
  
    struct DmaC
    {
	static constexpr uint32_t Mask = 0x8000ffff ;
	
	using Word = Neat::Bit::Word<uint32_t,Mask> ; 

	using Dreq   = Word::Set<0,8> ;
	using Panic  = Word::Set<8,8> ;
	using Enable = Word::Set<31,1> ;
	
	Word read() const { return Word::coset(*p) ; }

	void write(Word w) { (*p) = w.value() ; }

	static constexpr auto Permap = Dma::Ti::Permap::make<5>() ;
	
    private:
	
	friend Pwm ; uint32_t volatile *p ;

	DmaC(uint32_t volatile *p) : p(p) {}
    } ;

    DmaC dmaC() { return & page->at<0x8/4>() ; }
    
    Pwm(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x20c>())) {}
  
private:
  
    std::shared_ptr<Page> page ; 

} ; }

#endif // INCLUDE_Rpi_Pwm_h
