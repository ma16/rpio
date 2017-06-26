// BSD 2-Clause License, see github.com/ma16/rpio

#include "Memory.h"
#include "../Message.h"
#include "../../Error.h"
#include <cassert>

using Memory = Rpi::Mbox::Property::Memory ;

Memory::Mode Memory::Mode::deserialize(uint32_t u)
{
  Mode mode ;
  mode.discardable = 0 != (u & (1<<0)) ;
  mode.     direct = 0 != (u & (1<<2)) ;
  mode.   coherent = 0 != (u & (1<<3)) ;
  mode.       zero = 0 != (u & (1<<4)) ;
  mode.     noInit = 0 != (u & (1<<5)) ;
  mode.  permaLock = 0 != (u & (1<<6)) ;
  if (mode.serialize() != u)
    throw Error("Mbox::Property::Memory::Mode:invalid value") ;
  return mode ;
}

uint32_t Memory::Mode::serialize() const
{
  auto u =
    (static_cast<unsigned>(this->discardable) << 0) |
    (static_cast<unsigned>(this->     direct) << 2) |
    (static_cast<unsigned>(this->   coherent) << 3) |
    (static_cast<unsigned>(this->       zero) << 4) |
    (static_cast<unsigned>(this->     noInit) << 5) |
    (static_cast<unsigned>(this->  permaLock) << 6) ;
  return u ;
}

uint32_t Memory::allocate(Interface *iface,uint32_t size,uint32_t alignment,Mode mode)
{
  assert(alignment != 0) ;
  // ...crashes otherwise [future] type alignment
  Message const message({0x3000c,{size,alignment,mode.serialize()},4}) ;
  return iface->query(message,nullptr).front() ;
}

static uint32_t query(Rpi::Mbox::Interface *iface,uint32_t handle,uint32_t tag)
{
  Rpi::Mbox::Message const message({tag,{handle},4}) ;
  return iface->query(message,nullptr).front() ;
}

uint32_t Memory::   lock(Interface *iface,uint32_t handle) { return      query(iface,handle,0x3000d) ; }
bool     Memory:: unlock(Interface *iface,uint32_t handle) { return 0 == query(iface,handle,0x3000e) ; }
bool     Memory::release(Interface *iface,uint32_t handle) { return 0 == query(iface,handle,0x3000f) ; }
