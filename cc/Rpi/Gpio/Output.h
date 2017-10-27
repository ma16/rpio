// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Rpi_Gpio_Output_h
#define INCLUDE_Rpi_Gpio_Output_h

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 6: General Purpose I/O (GPIO)
// --------------------------------------------------------------------

#include "../Peripheral.h"
#include "../Pin.h" // [todo] should be in Gpio
#include "../Register.h"
#include <Neat/Numerator.h>

namespace Rpi { namespace Gpio {

struct Output
{
    static constexpr auto PNo = Peripheral::PNo::make<0x200>() ;
    
    Output(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;
    
    using Raise = Register::Word<Address+0x01c> ;
    using Clear = Register::Word<Address+0x028> ;
    
    Raise raise() { return Raise(&page->at<Raise::Offset/4>()) ; }
    Clear clear() { return Clear(&page->at<Clear::Offset/4>()) ; }

    // use as:
    //    raise().write(uint32_t) ;
    //    clear().write(uint32_t) ;
    
    enum class Level : unsigned { Hi=0,Lo=1 } ;
    // ...Hi is 0 since the Set register bank comes first
    // ...[todo] define string constants and lookup for UI

private:
    
    std::shared_ptr<Page> page ; 

} ; } }

#endif // INCLUDE_Rpi_Gpio_Output_h
