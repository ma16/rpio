// BSD 2-Clause License, see github.com/ma16/rpio

#include "Lib.h"
#include <Neat/stream.h>
#include <chrono>

void Console::Pwm::Lib::setup(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto c = pwm->getControl() ;
  c.clrf1() = 1 ; // may effect other channel
  auto x = c.get(index) ;
  x.pwen = 0 ;
  c.set(index,x) ;
  pwm->setControl(c) ;
  pwm->resetStatus(pwm->getStatus()) ;
  auto dmac = pwm->getDmac() ;
  dmac.enable = true ; // priority and dreq left unchanged
  pwm->setDmac(dmac) ;
}

void Console::Pwm::Lib::start(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  auto c = pwm->getControl() ;
  auto x = c.get(index) ;
  x.mode = 1 ; // serialize
  x.usef = 1 ;
  // sbit,pola,rptl left unchanged
  x.pwen = 1 ;
  c.set(index,x) ;
  pwm->setControl(c) ;
}

#include <Neat/cast.h>
void Console::Pwm::Lib::finish(Rpi::Pwm *pwm,Rpi::Pwm::Index index)
{
  while (0u == pwm->getStatus().cempt())
    ;
  auto dmac = pwm->getDmac() ;
  dmac.enable = false ; 
  pwm->setDmac(dmac) ;
  auto c = pwm->getControl() ;
  c.clrf1() = 1 ;
  auto x = c.get(index) ;
  x.pwen = 0 ;
  c.set(index,x) ;
  pwm->setControl(c) ;
}

double Console::Pwm::Lib::frequency(Rpi::Pwm *pwm,double duration)
{
  // backup pwm settings
  auto saved_ctrl = pwm->getControl() ;
  // prepare pwm test
  auto index = Rpi::Pwm::Index::make<0>() ;
  auto nbits = pwm->getRange(index) ;
  Rpi::Pwm::Control c ;
  // pwen = 0
  c.clrf1() = 1 ; 
  pwm->setControl(c) ;
  pwm->resetStatus(pwm->getStatus()) ;
  // fill queue
  while (0 == pwm->getStatus().cfull())
    pwm->write(0) ;
  // enable pwm
  c = Rpi::Pwm::Control() ;
  auto x = c.get(index) ;
  x.mode = 1 ; // serialize
  x.usef = 1 ; 
  x.sbit = 1 ; // (for debugging/monitoring and nbits>32)
  x.pwen = 1 ;
  c.set(index,x) ;
  using clock = std::chrono::steady_clock ;
  auto t0 = clock::now() ;
  pwm->setControl(c) ;
  // write pwm queue until time has passed
  using Seconds = std::chrono::duration<double,std::chrono::seconds::period> ;
  auto t1 = clock::now() ;
  auto count = 0u ;
  while (std::chrono::duration_cast<Seconds>(t1-t0).count() < duration) {
    auto status = pwm->getStatus() ;
    if (0 == status.cfull()) {
      if (0 != status.cempt())
	throw Error("fifo underrun") ;
      // [note] there is actually no safe way to detect an underrun
      pwm->write(0) ;
      ++count ;
    }
    t1 = clock::now() ;
  }
  auto freq = 0.0 ;
  // ...assume that no clock is running unless at least one entry has
  // been read from the queue (which should have happened even if d=0)
  if ((count>0) || (0 == pwm->getStatus().cfull())) {
    // get clock "exactly" after queue has read
    // ...well, "exactly" might be rather fuzzy in userland
    auto status = pwm->getStatus() ;
    if (0 == status.cfull()) {
      if (0 != status.cempt())
	throw Error("fifo underrun") ;
      pwm->write(0) ;
      ++count ;
    }
    while (0 != pwm->getStatus().cfull())
      ;
    t1 = clock::now() ;
    freq = static_cast<double>(nbits) * (count-1) / std::chrono::duration_cast<Seconds>(t1-t0).count() ;
  }
  // recover
  x.pwen = 0 ;
  c.set(index,x) ;
  c.clrf1() = 1 ;
  pwm->setControl(c) ;
  pwm->resetStatus(pwm->getStatus()) ;
  pwm->setControl(saved_ctrl) ;
  return freq ;
}

unsigned Console::Pwm::Lib::send(Rpi::Pwm pwm,Rpi::Pwm::Index index,uint32_t const data[],unsigned nwords)
{
  auto c = pwm.getControl() ;
  c.clrf1() = 1 ;
  auto x = c.get(index) ;
  x.pwen = 0 ;
  c.set(index,x) ;
  pwm.setControl(c) ;
  c.clrf1() = 0 ;
  pwm.resetStatus(pwm.getStatus()) ;

  // fill the PWM queue
  decltype(nwords) i = 0 ;
  while ((0 == pwm.getStatus().cfull()) && (i<nwords))
    pwm.write(data[i++]) ;
  // start serializer
  x.mode = 1 ; // serialize
  x.usef = 1 ; 
  x.pwen = 1 ;
  c.set(index,x) ;
  pwm.setControl(c) ;
  // top up the queue until all words have been written
  auto ngaps = 0u ;
  while (i<nwords) {
    auto status = pwm.getStatus() ;
    if (0 != status.cfull())
      continue ;
    if (0 != status.cempt())
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
  while (0 == pwm.getStatus().cempt())
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
  c.set(index,x) ;
  pwm.setControl(c) ;
  return ngaps ;
}

