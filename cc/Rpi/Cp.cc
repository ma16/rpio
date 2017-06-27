// BSD 2-Clause License, see github.com/ma16/rpio

#include "Cp.h"

void Rpi::Cp::enable(Index i) 
{
  auto c = getControl(i) ;
  c.enable = true ;
  c.kill = false ;
  set(i,c) ;
}

void Rpi::Cp::disable(Index i) 
{
  auto c = getControl(i) ;
  c.enable = false ;
  set(i,c) ;
  // the generator might still be busy (for a while)
}

void Rpi::Cp::kill(Index i) 
{ 
  auto c = getControl(i) ;
  c.kill = true ; set(i,c) ;
  while(busy(i)) ;
  c.kill = false ; set(i,c) ;
} 

void Rpi::Cp::set(Index i,Control::Source source,Divider::Intgr intgr,Divider::Fract fract,Control::Mash mash)
{
  auto c = getControl(i) ;
  // disable the generator if currently enabled
  auto e = c.enable ;
  if (e) {
    c.enable = false ; set(i,c) ;
    while (busy(i))
      ;
  }
  // set new value w/o enabling
  c.source = source ; c.mash = mash ; set(i,c) ;
  auto d = getDivider(i) ;
  d.intgr = intgr ; d.fract = fract ; set(i,d) ;
  // enable the generator if it was enabled in the first place
  if (e) {
    c.enable = e ; set(i,c) ;
  }
}