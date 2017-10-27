// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Pull_h
#define INCLUDE_Rpi_Gpio_Pull_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Pin.h" // [todo] should be in Gpio
#include "../Register.h"
#include <Neat/Numerator.h>

namespace Rpi { namespace Gpio {

struct Pull
{
    static constexpr auto PNo = Peripheral::PNo::make<0x200>() ;
    
    Pull(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;

    struct Control : Register::Masked<Address+0x094,0x3>
    {
	// Pull-up/down Register (GPPUD)
	using Base = Register::Masked<Address.value(),Mask> ;
	Control(Base base) : Base(base) {}
	using Mode = Word::Set<0,2> ;
	// ...[todo] writing 0x3 is not permitted
    } ;
    
    enum class Mode : unsigned { Off=0,Down=1,Up=2 } ;

    using ModeEnum = Neat::Numerator<Mode,Mode::Up> ;
    // ...[todo] define string constants and lookup for UI

    using Bank0 = Register::Word<Address+0x098> ; // pin  0-31
    using Bank1 = Register::Word<Address+0x09c> ; // pin 32-53

    Control control() { return Control(&page->at<Control::Offset/4>()) ; }
    Bank0     bank0() { return   Bank0(&page->at<Bank0  ::Offset/4>()) ; }
    Bank1     bank1() { return   Bank1(&page->at<Bank1  ::Offset/4>()) ; }
    
    void set(uint32_t set,Mode mode) ;
    // ...[todo] make set a type
    
    //void set(Pin      pin,Pull how) ;

private:
    
    std::shared_ptr<Page> page ; 

} ; } }

#endif // INCLUDE_Rpi_Gpio_Pull_h
