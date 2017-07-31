// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include <Neat/stream.h>
#include <chrono>

using Control = Rpi::Pwm::Control ;
using Status  = Rpi::Pwm:: Status ;

constexpr auto Channel1 = Rpi::Pwm::Index::make<0>() ;
constexpr auto Channel2 = Rpi::Pwm::Index::make<1>() ;

void Console::Pwm::Lib::setup(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto control = pwm->control().read() ;
  control.at(Control::Clrf) = 1 ;
  // ...may affect other channel too
  auto &b = Rpi::Pwm::Control::Bank::select(index) ;
  control.at(b.pwen) = 0 ;
  pwm->control().write(control) ;
  pwm->status().clear(pwm->status().read()) ;
  auto dmac = pwm->dmaC().read() ;
  dmac.enable = true ; // priority and dreq left unchanged
  pwm->dmaC().write(dmac) ;
}

void Console::Pwm::Lib::start(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto control = pwm->control().read() ;
  auto &b = Rpi::Pwm::Control::Bank::select(index) ;
  control.at(b.mode) = 1 ; // serialize
  control.at(b.usef) = 1 ;
  // sbit,pola,rptl are left unchanged
  control.at(b.pwen) = 1 ;
  pwm->control().write(control) ;
}

#include <Neat/cast.h>
void Console::Pwm::Lib::finish(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  while (!pwm->status().read().test(Status::Empt))
    ;
  auto dmac = pwm->dmaC().read() ;
  dmac.enable = false ; 
  pwm->dmaC().write(dmac) ;
  auto control = pwm->control().read() ;
  control.at(Control::Clrf) = 1 ;
  auto &b = Rpi::Pwm::Control::Bank::select(index) ;
  control.at(b.pwen) = 0 ;
  pwm->control().write(control) ;
}

unsigned Console::Pwm::Lib::send(Rpi::Pwm pwm,Rpi::Pwm::Index index,uint32_t const data[],unsigned nwords)
{
  auto control = pwm.control().read() ;
  control.at(Control::Clrf) = 1 ;
  auto &b = Rpi::Pwm::Control::Bank::select(index) ;
  control.at(b.pwen) = 0 ;
  pwm.control().write(control) ;
  control.at(Control::Clrf) = 0 ;
  pwm.status().clear(pwm.status().read()) ;

  // fill the PWM queue
  decltype(nwords) i = 0 ;
  while (!pwm.status().read().test(Status::Full) && (i<nwords))
    pwm.fifo().write(data[i++]) ;
  // start serializer
  control.at(b.mode) = 1 ; // serialize
  control.at(b.usef) = 1 ;
  control.at(b.pwen) = 1 ;
  pwm.control().write(control) ;
  // top up the queue until all words have been written
  auto ngaps = 0u ;
  while (i<nwords) {
    auto status = pwm.status().read() ;
    if (status.test(Status::Full))
      continue ;
    if (status.test(Status::Empt))
      ++ngaps ;
    // note:
    // --this might be a false positive if the serializer is still
    //   busy with the last queued word
    // --if a gap occurs right here, before wrting to the queue, we
    //   won't detect it. there is actually no indication by any status
    //   flag (BERR,GAPO,RERR) whether a gap occured -- even the spec
    //   says so.
    // [future] a time stamp at each iteration may be helpful to detect
    //   gaps
    //   (this would also need a prediction how long the queue may last)
    pwm.fifo().write(data[i]) ;
    ++i ;
  }
  // wait until the last word gets into the serializer
  while (!pwm.status().read().test(Status::Empt))
    ;
  // observed behavior:
  // --even though the last word of the queue has been read by the
  //   serializer, the transfer of the previous (!) word is still in
  //   progress. PWEN=0 at this point will interrupt the transfer in
  //   middle of the previous word.
  // --there is no indication by any status flag (BERR,GAPO,RERR) when
  //   the transfer is completed (even so the spec says so).
  // --after the transfer of the last word is completed, the transfer
  //   of the last word will be indefinitely repeated regardless
  //   whether the RPTL flag is set or not (spec is wrong again).
  //
  // hence, the caller should append two dummy word (i.e. all bits high
  // or all bits low, as needed) since this function will stop
  // transmission as soon as the queue gets empty.
  control.at(b.pwen) = 0 ;
  pwm.control().write(control) ;
  return ngaps ;
}

