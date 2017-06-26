// BSD 2-Clause License, see github.com/ma16/rpio

#include "Device.h"
#include "../Message.h"
#include "../../Error.h"
#include <Neat/cast.h>

using Device = Rpi::Mbox::Property::Device ;

bool Device::get_power_state(Interface* iface,Id id)
{
  Message const message(0x20001,{Neat::as_base(id)},8) ;
  auto payload = iface->query(message,nullptr) ;
  if (payload[0] != Neat::as_base(id))
    throw Error("Mbox::Property::Device:id does not match") ;
  if (0 != (payload[1] & 0x2))
    throw Error("Mbox::Property::Device:does not exist") ;
  return 0 != (payload[1] & 0x1) ;
}
  
bool Device::set_power_state(Interface* iface,Id id,bool on,bool wait)
{
  auto state = (static_cast<uint32_t>(wait)<<1) | static_cast<uint32_t>(on) ;
  Message const message(0x28001,{Neat::as_base(id),state},8) ;
  auto payload = iface->query(message,nullptr) ;
  if (payload[0] != Neat::as_base(id))
    throw Error("Mbox::Property::Device:id does not match") ;
  if (0 != (payload[1] & 0x2))
    throw Error("Mbox::Property::Device:does not exist") ;
  return 0 != (payload[1] & 0x1) ;
}

uint32_t Device::timing(Interface* iface,Id id)
{
  Message const message(0x20002,{Neat::as_base(id)},8) ; 
  auto payload = iface->query(message,nullptr) ;
  if (payload[0] != Neat::as_base(id))
    throw Error("Mbox::Property::Device:id does not match") ;
  return payload[1] ;
}

char const *Device::name(Id id)
{
  static char const * const array[] = { "sdcard","uart0","uart1","usbhcd","i2c0","i2c1","i2c2","spi","ccp2tx" } ;
  static_assert(Neat::as_base(Id::CCP2TX)+1 == sizeof(array)/sizeof(array[0]),"") ;
  return array[Neat::as_base(id)] ;
}
