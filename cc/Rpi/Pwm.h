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
	
	static constexpr auto Pwen1 = Word::Digit::make< 0>() ; 
	static constexpr auto Mode1 = Word::Digit::make< 1>() ; 
	static constexpr auto Rptl1 = Word::Digit::make< 2>() ; 
	static constexpr auto Sbit1 = Word::Digit::make< 3>() ; 
	static constexpr auto Pola1 = Word::Digit::make< 4>() ; 
	static constexpr auto Usef1 = Word::Digit::make< 5>() ; 
	static constexpr auto Clrf  = Word::Digit::make< 6>() ; 
	static constexpr auto Msen1 = Word::Digit::make< 7>() ; 
	static constexpr auto Pwen2 = Word::Digit::make< 8>() ; 
	static constexpr auto Mode2 = Word::Digit::make< 9>() ; 
	static constexpr auto Rptl2 = Word::Digit::make<10>() ; 
	static constexpr auto Sbit2 = Word::Digit::make<11>() ; 	
	static constexpr auto Pola2 = Word::Digit::make<12>() ; 
	static constexpr auto Usef2 = Word::Digit::make<13>() ; 
	static constexpr auto Msen2 = Word::Digit::make<15>() ;
	
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

	static constexpr auto Full = Word::Digit::make< 0>() ; 
	static constexpr auto Empt = Word::Digit::make< 1>() ; 
	static constexpr auto Werr = Word::Digit::make< 2>() ; 
	static constexpr auto Rerr = Word::Digit::make< 3>() ; 
	static constexpr auto Gap1 = Word::Digit::make< 4>() ; 
	static constexpr auto Gap2 = Word::Digit::make< 5>() ; 
	static constexpr auto Berr = Word::Digit::make< 8>() ; 
	static constexpr auto Sta1 = Word::Digit::make< 9>() ; 
	static constexpr auto Sta2 = Word::Digit::make<10>() ; 

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
	struct Word
	{
	    bool  enable  ; 
	    uint8_t panic ; 
	    uint8_t dreq  ; 

	    static Word make(uint32_t r) ; uint32_t value() const ;
	    
	private:
	    Word(bool enable,uint8_t panic,uint8_t dreq)
		: enable(enable),panic(panic),dreq(dreq) {}
	} ;
	
	Word read() const { return Word::make(*p) ; }

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
