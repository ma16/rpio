// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// PWM is used in Poll mode (no DMA) which might even work if the
// process is de-scheduled for just a brief moment in time.
//
// A straight forward approach is prone to undetected fifo underruns.
//
// A FIFO underrun may occur:
// * if data is faster read from FIFO than written to
// * if the writing thread gets suspended
//
// You may check if the FIFO is empty to detect an underrun. However,
// there are (less likely but still probable) cases, when an underrun
// goes undected (i.e. immediately if the underrun occurs immediately
// after the call that tests whether the FIFO is empty).
//
// You cannot prevent an underrun. However, you can detect it: The
// FIFO can hold a maximum of 16 words. So if you write 16 words to the
// FIFO w/o that the FIFO is getting full, there might have been a
// FIFO underrun. So, using this indicator, you may also get false
// positives.
// --------------------------------------------------------------------

#ifndef INCLUDE_RpiExt_Pwm_h
#define INCLUDE_RpiExt_Pwm_h

#include <Rpi/Timer.h>
#include <Rpi/Pwm.h>
#include <vector>

namespace RpiExt {

struct Pwm
{
    struct Error : Neat::Error
    {
      Error(std::string const &s) : Neat::Error("RpiExt:Pwm:" + s) {}
    } ;

    // block until written (possible underrun)
    void write(uint32_t const buffer[],size_t nwords) ;

    // write until blocking (possible underrun)
    size_t topUp(uint32_t const buffer[],size_t nwords) ;

    // return number of bytes before underrun
    size_t convey(uint32_t const buffer[],size_t nwords,uint32_t pad) ; 

    // ----
    
    // guess frequency by flooding the FIFO for the given duration
    double frequency(Rpi::Pwm::Index index,double duration) ;
    // pwen and other values have to set-up by the client beforehand!

    // ----
    
    Pwm(Rpi::Peripheral *rpi) : timer(rpi),pwm(rpi) {}
    
private:
  
    Rpi::Timer timer ; Rpi::Pwm pwm ;

    // write n x word to fifo and return the werr-flag
    bool fillUp(size_t n,uint32_t word) ;
    
    // return true if fifo is not full (within given timeout)
    bool wait(uint32_t timeout) ;

    // [todo] reset/recover if destructed?
    
} ; }

#endif // INCLUDE_RpiExt_Pwm_h
