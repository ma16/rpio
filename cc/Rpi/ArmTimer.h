// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: Chapter 14: Timer (ARM Side)
//
// Notes:
// "The clock from the ARM timer is derived from the system clock. This
//  clock can change dynamically e.g. if the system goes into reduced
//  power or in low power mode. Thus the clock speed adapts to the
//  overal system performance capabilities. For accurate timing it is
//  recommended to use the system timers."
// --------------------------------------------------------------------

#ifndef INCLUDE_Rpi_ArmTimer_h
#define INCLUDE_Rpi_ArmTimer_h

#include "Peripheral.h"
#include "Register.h"

namespace Rpi {

struct ArmTimer
{
    static constexpr auto PNo = Peripheral::PNo::make<0x00b>() ;
    
    ArmTimer(Peripheral *p) : page(p->page(PNo)) {}
  
    static constexpr auto Address =
    Bus::Address::Base + PNo.value() * Page::nbytes ;

    struct Control : Register::Masked<Address+0x408,0x00ff0200>
    {
	using Base = Register::Masked<Address.value(),Mask> ;
	Control(Base base) : Base(base) {}
	using Enabled = Word::Bit< 9  > ; 
	using Divider = Word::Set<16,8> ; // "pre-scaler" 1..256
    } ;

    Control control() { return Control(&page->at<Control::Offset/4>()) ; }
    
    using Counter = Register::Word<Address+0x420> ;

    Counter counter() const { return Counter(&page->at<Counter::Offset/4>()) ; }
    // [todo] should return a const reference / pointer

    double frequency() const ;
    
private:

    std::shared_ptr<Page> page ;

} ; }

// [todo]
// introduce chrono::clock-like implementation for the two Pi clocks
// (i.e. for the BCM timer and the ARM counter). ponder the effects of
// clock resolution (accuracy), granularity and cost-per-call. at least
// introduce wrapper classes for time-point and duration instaed of
// using ambiguous uint32_t types.

#endif // INCLUDE_Rpi_ArmTimer_h
