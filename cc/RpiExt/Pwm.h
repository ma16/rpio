// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// PWM can be employed in DMA-, interrupt- or Poll mode.
//
// Here, PWM is used in Poll mode. That is, the FIFO is topped-up
// whenever there is space until all data has been written. The status
// of the FIFO is polled in a busy loop to detect whether there is
// space or not.
//
// All the functions here assume that PWM is properly set-up. That
// includes an enabled serializer (PWEN=1). The functions operate only
// on the FIFO and affect/query/reset the status flags. They do not
// operate on channel-specific registers (unless provided as function
// argument).
//
// The Poll mode is prone to undetected FIFO underruns. The FIFO may
// run out of data (unnoticed) in the middle of a transmission.
// Imagine you check the status of the FIFO (which says: not-empty) and
// after that the writing thread gets suspended. If the suspension
// lasts long enough, the FIFO runs empty. This is a less likely, but
// still probable scenario.
//
// In general, a FIFO underrun may occur:
// * if the serializer operates faster than (uninterrupted) FIFO top-up
//   operations
// * if the thread that top-ups the FIFO gets interrupted
//
// Since the FIFO can hold a maximum of 16 words, you can detect a FIFO
// underrun by "block-writes" of 16 words. If the FIFO isn't full
// after that, there might have been a FIFO underrun. (It might also be
// a false positive.)
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

    // block until all data has been written (undetected underruns)
    void write(uint32_t const buffer[],size_t nwords) ;

    // write partial data until FIFO full (undetected underruns)
    size_t topUp(uint32_t const buffer[],size_t nwords) ;

    // block until all data has been written; return early on underrun
    size_t convey(uint32_t const buffer[],size_t nwords,uint32_t pad) ;
    // the data will be pre/postfixed by multiple words (pad)

    // guess the frequency for the given duration
    std::pair<double,size_t> measureRate(double duration) ;
    // returns (words per seconds,number of potential underruns)
    
    Pwm(Rpi::Peripheral *rpi) : timer(rpi),pwm(rpi) {}
    
private:
  
    Rpi::Timer timer ; Rpi::Pwm pwm ;

    // write n x word to fifo and return the werr-flag
    bool fillUp(size_t n,uint32_t word) ;
    
    // return true if FIFO gets writable (timeout in milliseconds)
    bool writable(uint32_t timeout) ;
    
} ; }

#endif // INCLUDE_RpiExt_Pwm_h
