// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Function_h
#define INCLUDE_Rpi_Gpio_Function_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Pin.h" // [todo] should be in Gpio
#include "../Register.h"
#include <Neat/Numerator.h>
// [todo] merge with Gpio::Function

namespace Rpi { namespace Gpio { struct Function
{
    static constexpr auto PNo = Peripheral::PNo::make<0x200>() ;
    
    Function(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;
    
    // GPIO Alternate function select register 0 (GPFSEL0) 
    struct Bank0 : Register::Masked<Address+0x000,0x3fffffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank0(Base base) : Base(base) {}
	using Pin0 = Word::Set< 0,3> ;
	using Pin1 = Word::Set< 3,3> ;
	using Pin2 = Word::Set< 6,3> ;
	using Pin3 = Word::Set< 9,3> ;
	using Pin4 = Word::Set<12,3> ;
	using Pin5 = Word::Set<15,3> ;
	using Pin6 = Word::Set<18,3> ;
	using Pin7 = Word::Set<21,3> ;
	using Pin8 = Word::Set<24,3> ;
	using Pin9 = Word::Set<27,3> ;
    } ;

    // GPIO Alternate function select register 1 (GPFSEL1) 
    struct Bank1 : Register::Masked<Address+0x004,0x3fffffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank1(Base base) : Base(base) {}
	using Pin10 = Word::Set< 0,3> ;
	using Pin11 = Word::Set< 3,3> ;
	using Pin12 = Word::Set< 6,3> ;
	using Pin13 = Word::Set< 9,3> ;
	using Pin14 = Word::Set<12,3> ;
	using Pin15 = Word::Set<15,3> ;
	using Pin16 = Word::Set<18,3> ;
	using Pin17 = Word::Set<21,3> ;
	using Pin18 = Word::Set<24,3> ;
	using Pin19 = Word::Set<27,3> ;
    } ;
    
    // GPIO Alternate function select register 2 (GPFSEL2) 
    struct Bank2 : Register::Masked<Address+0x008,0x3fffffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank2(Base base) : Base(base) {}
	using Pin20 = Word::Set< 0,3> ;
	using Pin21 = Word::Set< 3,3> ;
	using Pin22 = Word::Set< 6,3> ;
	using Pin23 = Word::Set< 9,3> ;
	using Pin24 = Word::Set<12,3> ;
	using Pin25 = Word::Set<15,3> ;
	using Pin26 = Word::Set<18,3> ;
	using Pin27 = Word::Set<21,3> ;
	using Pin28 = Word::Set<24,3> ;
	using Pin29 = Word::Set<27,3> ;
    } ;
    
    // GPIO Alternate function select register 30 (GPFSEL3) 
    struct Bank3 : Register::Masked<Address+0x00c,0x3fffffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank3(Base base) : Base(base) {}
	using Pin30 = Word::Set< 0,3> ;
	using Pin31 = Word::Set< 3,3> ;
	using Pin32 = Word::Set< 6,3> ;
	using Pin33 = Word::Set< 9,3> ;
	using Pin34 = Word::Set<12,3> ;
	using Pin35 = Word::Set<15,3> ;
	using Pin36 = Word::Set<18,3> ;
	using Pin37 = Word::Set<21,3> ;
	using Pin38 = Word::Set<24,3> ;
	using Pin39 = Word::Set<27,3> ;
    } ;
    
    // GPIO Alternate function select register 4 (GPFSEL4) 
    struct Bank4: Register::Masked<Address+0x010,0x3fffffff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank4(Base base) : Base(base) {}
	using Pin40 = Word::Set< 0,3> ;
	using Pin41 = Word::Set< 3,3> ;
	using Pin42 = Word::Set< 6,3> ;
	using Pin43 = Word::Set< 9,3> ;
	using Pin44 = Word::Set<12,3> ;
	using Pin45 = Word::Set<15,3> ;
	using Pin46 = Word::Set<18,3> ;
	using Pin47 = Word::Set<21,3> ;
	using Pin48 = Word::Set<24,3> ;
	using Pin49 = Word::Set<27,3> ;
    } ;
    
    // GPIO Alternate function select register 5 (GPFSEL5) 
    struct Bank5 : Register::Masked<Address+0x014,0xfff>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Bank5(Base base) : Base(base) {}
	using Pin50 = Word::Set< 0,3> ;
	using Pin51 = Word::Set< 3,3> ;
	using Pin52 = Word::Set< 6,3> ;
	using Pin53 = Word::Set< 9,3> ;
    } ;

    Bank0 bank0() { return Bank0(&page->at<Bank0::Offset/4>()) ; }
    Bank1 bank1() { return Bank1(&page->at<Bank1::Offset/4>()) ; }
    Bank2 bank2() { return Bank2(&page->at<Bank2::Offset/4>()) ; }
    Bank3 bank3() { return Bank3(&page->at<Bank3::Offset/4>()) ; }
    Bank4 bank4() { return Bank4(&page->at<Bank4::Offset/4>()) ; }
    Bank5 bank5() { return Bank5(&page->at<Bank5::Offset/4>()) ; }
    
    enum class Type : unsigned
    { In=0,Out=1,Alt5=2,Alt4=3,Alt0=4,Alt1=5,Alt2=6,Alt3=7 } ;

    using Numerator = Neat::Numerator<Type,Type::Alt3> ;


    
    Type get(Pin pin) const ;
    void set(Pin pin,Type mode) ;

    void set(uint32_t set,Type mode) ;

private:

    std::shared_ptr<Page> page ;
  
} ; } }

#endif // INCLUDE_Rpi_Gpio_Function_h
