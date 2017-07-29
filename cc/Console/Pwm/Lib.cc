// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include <Neat/stream.h>
#include <chrono>

void Console::Pwm::Lib::setup(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto control = pwm->getControl() ;
  control.clear = 1 ; // may affect other channel
  auto &x = control.channel[index.value()] ;
  x.pwen = 0 ;
  pwm->setControl(control) ;
  pwm->clearStatus(pwm->getStatus()) ;
  auto dmac = pwm->getDmac() ;
  dmac.enable = true ; // priority and dreq left unchanged
  pwm->setDmac(dmac) ;
}

void Console::Pwm::Lib::start(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto control = pwm->getControl() ;
  auto &x = control.channel[index.value()] ;
  x.mode = 1 ; // serialize
  x.usef = 1 ;
  // sbit,pola,rptl are left unchanged
  control.channel[index.value()].pwen = 1 ;
  pwm->setControl(control) ;
}

#include <Neat/cast.h>
void Console::Pwm::Lib::finish(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  while (0u == pwm->getStatus().empt)
    ;
  auto dmac = pwm->getDmac() ;
  dmac.enable = false ; 
  pwm->setDmac(dmac) ;
  auto control = pwm->getControl() ;
  control.clear = 1 ;
  auto &x = control.channel[index.value()] ;
  x.pwen = 0 ;
  pwm->setControl(control) ;
}

unsigned Console::Pwm::Lib::send(Rpi::Pwm pwm,Rpi::Pwm::Index index,uint32_t const data[],unsigned nwords)
{
  auto control = pwm.getControl() ;
  control.clear = 1 ;
  auto &x = control.channel[index.value()] ;
  x.pwen = 0 ;
  pwm.setControl(control) ;
  control.clear = 0 ;
  pwm.clearStatus(pwm.getStatus()) ;

  // fill the PWM queue
  decltype(nwords) i = 0 ;
  while ((0 == pwm.getStatus().full) && (i<nwords))
    pwm.write(data[i++]) ;
  // start serializer
  x.mode = 1 ; // serialize
  x.usef = 1 ; 
  x.pwen = 1 ;
  pwm.setControl(control) ;
  // top up the queue until all words have been written
  auto ngaps = 0u ;
  while (i<nwords) {
    auto status = pwm.getStatus() ;
    if (0 != status.full)
      continue ;
    if (0 != status.empt)
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
    pwm.write(data[i]) ;
    ++i ;
  }
  // wait until the last word gets into the serializer
  while (0 == pwm.getStatus().empt)
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
  x.pwen = 0 ;
  pwm.setControl(control) ;
  return ngaps ;
}

