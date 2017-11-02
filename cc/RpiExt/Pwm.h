// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// PWM can be employed in DMA or Poll mode. Here, PWM is used in Poll
// mode only. That is, the FIFO is topped-up whenever there is space
// until all data has been written. The status of the FIFO is polled
// in a busy loop to detect whether there is space or not.
//
// All the functions here assume that PWM is properly set-up. That
// includes an enabled peripheral (PWEN=1).

// All the functions operate only on the FIFO and affect/query/reset
// the status flags. They do not operate on channel-specific registers.
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Pwm_h
#define INCLUDE_RpiExt_Pwm_h

#include <Rpi/Register.h>
#include <Rpi/Pwm.h>
#include <Rpi/Timer.h>

namespace RpiExt {

struct Pwm
{
    struct Error : Neat::Error
    {
      Error(std::string const &s) : Neat::Error("RpiExt:Pwm:" + s) {}
    } ;

    // block until all data has been written; return early on underrun
    size_t convey(uint32_t const buffer[],size_t nwords,uint32_t pad) ;
    // the data will be postfixed by multiple words (pad)
    // the FIFO must not be empty when called (use headstart or fillUp)

    // write n x word to fifo and return the werr-flag
    bool fillUp(size_t n,uint32_t word) ;
    
    // interrupt transmission for topping-up the FIFO
    size_t headstart(uint32_t const buffer[],size_t nwords) ;
		     
    // guess the frequency for the given duration
    std::pair<double,size_t> measureRate(double duration) ;
    // returns (words per seconds,number of potential underruns)
    
    // write partial data until FIFO full (undetected underruns)
    size_t topUp(uint32_t const buffer[],size_t nwords) ;

    // return true if FIFO gets writable (timeout in milliseconds)
    bool writable(uint32_t timeout) ;
  
    // block until all data has been written (undetected underruns)
    void write(uint32_t const buffer[],size_t nwords) ;

    // set control register and repeat until BERR=0
    void setControl(Rpi::Pwm::Control::Word) ;

    Pwm(Rpi::Peripheral *rpi)
	: timer(rpi),pwm(rpi)
	, base(rpi->page<Rpi::Register::Pwm::PageNo>()) {}
    
private:
  
    Rpi::Timer timer ; Rpi::Pwm pwm ;

    Rpi::Register::Base<Rpi::Register::Pwm::PageNo> base ;

} ; }

#endif // INCLUDE_RpiExt_Pwm_h
