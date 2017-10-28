// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Input_h
#define INCLUDE_Rpi_Gpio_Input_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Pin.h" // [todo] should be in Gpio
#include "../Register.h"
#include <Neat/Numerator.h>

namespace Rpi { namespace Gpio {

struct Input
{
    static constexpr auto PNo = Peripheral::PNo::make<0x200>() ;
    
    Input(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;
    
    using Bank0 = Register::Word  <Address+0x034         > ; // GPLEV0: pins  0-31
    using Bank1 = Register::Masked<Address+0x038,0x3fffff> ; // GPLEV1: pins 32-53

    Bank0 bank0() { return Bank0(&page->at<Bank0::Offset/4>()) ; }
    Bank1 bank1() { return Bank1(&page->at<Bank1::Offset/4>()) ; }
    
    // use as bank().read() ;

private:
    
    std::shared_ptr<Page> page ; 

} ; } }

#endif // INCLUDE_Rpi_Gpio_Input_h
