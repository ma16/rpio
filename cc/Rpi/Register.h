// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Register_h
#define INCLUDE_Rpi_Register_h

#include <Neat/Bit/Digit.h>
#include <Neat/Bit/Set.h>
#include <Neat/Bit/Word.h>
#include <Neat/Enum.h>
#include <cstdint>

namespace Rpi { class Peripheral ; }

namespace Rpi { namespace Register
{
    template<uint32_t P> struct Base ;
	
    using Pno = Neat::Enum<unsigned,0x1000-1> ; // page number
    using Pix = Neat::Enum<unsigned, 0x400-1> ; // word index within page
  
    template<uint32_t P,uint32_t O,uint32_t R,uint32_t W> 
    struct Traits 
    {
	static constexpr Pno PageNo = Pno::make<P>() ;
	static constexpr auto Index = Pix::make<O/4>() ;
	// ...the (byte) offset is converted to the word offset
	
	using  ReadWord = Neat::Bit::Word<uint32_t,R> ;
	using WriteWord = Neat::Bit::Word<uint32_t,W> ;
	
	static constexpr auto  ReadMask = R ; 
	static constexpr auto WriteMask = W ;
	// static Bus::Address address = 0x7e<Pno><Ofs> 
    } ;

    template<uint32_t P,uint32_t O,uint32_t R,uint32_t W>
    constexpr Pno Traits<P,O,R,W>::PageNo ;

    template<typename Traits> struct Pointer
    {
	typename Traits::ReadWord read() const
	{
	    using ReadWord = typename Traits::ReadWord ;
	    return ReadWord::coset(*p) ;
	}

	void write(typename Traits::WriteWord w)
	{
	    (*p) = w.value() ;
	}
	
	uint32_t volatile* value() { return p ; }
	
    private:

	friend class Base<Traits::PageNo.value()> ; 

	friend class Rpi::Peripheral ;
	
	volatile uint32_t *p ; Pointer(volatile uint32_t *p) : p(p) {}
    } ;

    template<uint32_t P> struct Base 
    {
	template<typename Traits> Pointer<Traits> at()
	{
	    static_assert(P == Traits::PageNo.value(),"") ;
	    return Pointer<Traits>(p + Traits::Index.value()) ;
	}

	uint32_t volatile      * ptr()       { return p ; }
	uint32_t volatile const* ptr() const { return p ; }
	
    private:

	friend class Rpi::Peripheral ;
	
	volatile uint32_t *p ; Base(volatile uint32_t *p) : p(p) {}
    } ;

    template<typename U,unsigned O> using Digit = Neat::Bit::Digit<U,O> ;
    
    template<typename U,unsigned O,unsigned L> using Set = Neat::Bit::Set<U,O,L> ;
    
    // BCM2835 ARM Peripherals §6: General Purpose I/O (GPIO)

    namespace Gpio
    {
	static constexpr auto PageNo = 0x200u ;

	namespace Function
	{
	    using Bank0 = Traits<PageNo,0x00,0x3fffffff,0x3fffffff> ; // GPFSEL0
	    using Bank1 = Traits<PageNo,0x04,0x3fffffff,0x3fffffff> ; // GPFSEL1
	    using Bank2 = Traits<PageNo,0x08,0x3fffffff,0x3fffffff> ; // GPFSEL2
	    using Bank3 = Traits<PageNo,0x0c,0x3fffffff,0x3fffffff> ; // GPFSEL3
	    using Bank4 = Traits<PageNo,0x10,0x3fffffff,0x3fffffff> ; // GPFSEL4
	    using Bank5 = Traits<PageNo,0x14,     0xfff,     0xfff> ; // GPFSEL5
	    
	    enum class Type : unsigned
	    { In=0,Out=1,Alt5=2,Alt4=3,Alt0=4,Alt1=5,Alt2=6,Alt3=7 } ;
	}

	namespace Output
	{
	    using Raise0 = Traits<PageNo,0x1c,0x0,0xffffffff> ; // GPSET0
	    using Raise1 = Traits<PageNo,0x20,0x0,  0x3fffff> ; // GPSET1
	    using Clear0 = Traits<PageNo,0x28,0x0,0xffffffff> ; // GPCLR0
	    using Clear1 = Traits<PageNo,0x2c,0x0,  0x3fffff> ; // GPCLR1
	    
	    enum class Level : unsigned { Hi=0,Lo=1 } ;
	}
	
	namespace Input
	{
	    using Bank0 = Traits<PageNo,0x34,0xffffffff,0x0> ; // GPLEV0
	    using Bank1 = Traits<PageNo,0x38,  0x3fffff,0x0> ; // GPLEV1
	}

	namespace Event
	{
	    using Status0    = Traits<PageNo,0x40,0xffffffff,0xffffffff> ; // GPEDS0 
	    using Status1    = Traits<PageNo,0x44,  0x3fffff,  0x3fffff> ; // GPEDS1 
	    using Rise0      = Traits<PageNo,0x4c,0xffffffff,0xffffffff> ; // GPREN0
	    using Rise1      = Traits<PageNo,0x50,  0x3fffff,  0x3fffff> ; // GPREN1
	    using Fall0      = Traits<PageNo,0x58,0xffffffff,0xffffffff> ; // GPFEN0
	    using Fall1      = Traits<PageNo,0x5c,  0x3fffff,  0x3fffff> ; // GPFEN1
	    using High0      = Traits<PageNo,0x64,0xffffffff,0xffffffff> ; // GPHEN0
	    using High1      = Traits<PageNo,0x68,  0x3fffff,  0x3fffff> ; // GPHEN1
	    using Low0       = Traits<PageNo,0x70,0xffffffff,0xffffffff> ; // GPLEN0
	    using Low1       = Traits<PageNo,0x74,  0x3fffff,  0x3fffff> ; // GPLEN1
	    using AsyncRise0 = Traits<PageNo,0x7c,0xffffffff,0xffffffff> ; // GPAREN0
	    using AsyncRise1 = Traits<PageNo,0x80,  0x3fffff,  0x3fffff> ; // GPAREN1
	    using AsyncFall0 = Traits<PageNo,0x88,0xffffffff,0xffffffff> ; // GPAFEN0
	    using AsyncFall1 = Traits<PageNo,0x8c,  0x3fffff,  0x3fffff> ; // GPAFEN1
	    
	    enum class Type : unsigned
	    { Rise=0,Fall=1,High=2,Low=3,AsyncRise=4,AsyncFall=5 } ;
	}

	namespace Pull
	{
	    using Control = Traits<PageNo,0x94,0x0,       0x3> ;
	    using Bank0   = Traits<PageNo,0x98,0x0,0xffffffff> ;
	    using Bank1   = Traits<PageNo,0x9c,0x0,0xffffffff> ;
	}
    }
    
    // BCM2835 ARM Peripherals §9: Pulse Width Modulator

    namespace Pwm
    {
	static constexpr auto PageNo = 0x20cu ;

	using Control = Traits<PageNo,0x00,0xbfff,0xbfff> ;

	using Pwen1 = Digit<uint32_t, 0> ;
	using Mode1 = Digit<uint32_t, 1> ; 
	using Rptl1 = Digit<uint32_t, 2> ; 
	using Sbit1 = Digit<uint32_t, 3> ; 
	using Pola1 = Digit<uint32_t, 4> ; 
	using Usef1 = Digit<uint32_t, 5> ; 
	using Clrf  = Digit<uint32_t, 6> ; 
	using Msen1 = Digit<uint32_t, 7> ; 
	using Pwen2 = Digit<uint32_t, 8> ; 
	using Mode2 = Digit<uint32_t, 9> ; 
	using Rptl2 = Digit<uint32_t,10> ; 
	using Sbit2 = Digit<uint32_t,11> ; 	
	using Pola2 = Digit<uint32_t,12> ; 
	using Usef2 = Digit<uint32_t,13> ; 
	using Msen2 = Digit<uint32_t,15> ;

	using Status = Traits<PageNo,0x04,0x73f,0x73f> ;

	using Full = Digit<uint32_t, 0> ; 
	using Empt = Digit<uint32_t, 1> ; 
	using Werr = Digit<uint32_t, 2> ; 
	using Rerr = Digit<uint32_t, 3> ; 
	using Gap1 = Digit<uint32_t, 4> ; 
	using Gap2 = Digit<uint32_t, 5> ; 
	using Berr = Digit<uint32_t, 8> ; 
	using Sta1 = Digit<uint32_t, 9> ; 
	using Sta2 = Digit<uint32_t,10> ; 

	using DmaC = Traits<PageNo,0x08,0x8000ffff,0x8000ffff> ;

	using Dreq   = Set<uint32_t, 0,8> ;
	using Panic  = Set<uint32_t, 8,8> ;
	using Enable = Set<uint32_t,31,1> ;

	using Range1 = Traits<PageNo,0x10,0xffffffff,0xffffffff> ;
	using Data1  = Traits<PageNo,0x14,0xffffffff,0xffffffff> ;
	using Fifo   = Traits<PageNo,0x18,0xffffffff,0xffffffff> ;
	using Range2 = Traits<PageNo,0x20,0xffffffff,0xffffffff> ;
	using Data2  = Traits<PageNo,0x24,0xffffffff,0xffffffff> ;
    }

} }

#endif // INCLUDE_Rpi_Register_h
