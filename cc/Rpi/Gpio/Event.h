// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Event_h
#define INCLUDE_Rpi_Gpio_Event_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Pin.h" // [todo] should be in Gpio
#include "../Register.h"
#include <Neat/Numerator.h>

namespace Rpi { namespace Gpio {

struct Event
{
    static constexpr auto PNo = Peripheral::PNo::make<0x200>() ;
    
    Event(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;

    using    Status0 = Register::Word<Address+0x040> ; // GPEDS0
    using      Rise0 = Register::Word<Address+0x04c> ; // GP-R -EN0
    using      Fall0 = Register::Word<Address+0x058> ; // GP-F -EN0
    using      High0 = Register::Word<Address+0x064> ; // GP-H -EN0
    using       Low0 = Register::Word<Address+0x070> ; // GP-L -EN0
    using AsyncRise0 = Register::Word<Address+0x07c> ; // GP-AR-EN0
    using AsyncFall0 = Register::Word<Address+0x088> ; // GP-AF-EN0

    Status0       status0() { return Status0   (&page->at<   Status0::Offset/4>()) ; }
    Rise0           rise0() { return Rise0     (&page->at<     Rise0::Offset/4>()) ; }
    Fall0           fall0() { return Fall0     (&page->at<     Fall0::Offset/4>()) ; }
    High0           high0() { return High0     (&page->at<     High0::Offset/4>()) ; }
    Low0             low0() { return Low0      (&page->at<      Low0::Offset/4>()) ; }
    AsyncRise0 asyncRise0() { return AsyncRise0(&page->at<AsyncRise0::Offset/4>()) ; }
    AsyncFall0 asyncFall0() { return AsyncFall0(&page->at<AsyncFall0::Offset/4>()) ; }
    
    using    Status1 = Register::Masked<Address+0x044,0x3fffff> ; // GPEDS1
    using      Rise1 = Register::Masked<Address+0x050,0x3fffff> ; // GP-R -EN1
    using      Fall1 = Register::Masked<Address+0x05c,0x3fffff> ; // GP-F -EN1
    using      High1 = Register::Masked<Address+0x068,0x3fffff> ; // GP-H -EN1
    using       Low1 = Register::Masked<Address+0x07c,0x3fffff> ; // GP-L -EN1
    using AsyncRise1 = Register::Masked<Address+0x090,0x3fffff> ; // GP-AR-EN1
    using AsyncFall1 = Register::Masked<Address+0x08c,0x3fffff> ; // GP-AF-EN1

    Status1       status1() { return Status1   (&page->at<   Status1::Offset/4>()) ; }
    Rise1           rise1() { return Rise1     (&page->at<     Rise1::Offset/4>()) ; }
    Fall1           fall1() { return Fall1     (&page->at<     Fall1::Offset/4>()) ; }
    High1           high1() { return High1     (&page->at<     High1::Offset/4>()) ; }
    Low1             low1() { return Low1      (&page->at<      Low1::Offset/4>()) ; }
    AsyncRise1 asyncRise1() { return AsyncRise1(&page->at<AsyncRise1::Offset/4>()) ; }
    AsyncFall1 asyncFall1() { return AsyncFall1(&page->at<AsyncFall1::Offset/4>()) ; }

    enum class Type : unsigned { Rise=0,Fall=1,High=2,Low=3,AsyncRise=4,AsyncFall=5 } ;

    using TypeEnum = Neat::Numerator<Type,Type::AsyncFall> ;
    
    uint32_t enable(uint32_t set,Type type,bool on)
    {
	// returns the previous set
	auto i = 0x4c + TypeEnum(type).n() * 0xc ;
	// 0x4c : GP-R -EN0 
	// 0x58 : GP-F -EN0 
	// 0x64 : GP-H -EN0
	// 0x70 : GP-L -EN0 
	// 0x7c : GP-AR-EN0 
	// 0x88 : GP-AF-EN0
	return rmw(page->at(Page::Index::make(i/4)),set,on) ; 
    }

    bool enable(Pin pin,Type type,bool on)
    {
	auto mask = 1u << pin.value() ;
	auto word = enable(mask,type,on) ;
	return 0 != (word & mask) ;
    }
    
private:
    
    std::shared_ptr<Page> page ; 

    static uint32_t rmw(uint32_t volatile &r,uint32_t set,bool on) ;
  
} ; } }

#endif // INCLUDE_Rpi_Gpio_Event_h
