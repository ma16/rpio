// BSD 2-Clause License, see github.com/ma16/rpio

#include "Ram.h"
#include "../Message.h"
#include "../../Error.h"

namespace Property = Rpi::Mbox::Property ;

static std::vector<Property::Ram> query(Rpi::Mbox::Interface *iface,uint32_t tag) 
{
  static Rpi::Mbox::Message const probe({tag,{},0}) ;
  uint32_t nbytes ;
  iface->query(probe,&nbytes) ;
  auto size = nbytes / 8 ;
  if (8u*size != nbytes)
    throw Rpi::Error("Mbox::Property::Ram:unexpected size") ;
  Rpi::Mbox::Message const message({tag,{},nbytes}) ;
  auto payload = iface->query(message,nullptr) ;
  std::vector<Property::Ram> v ; v.reserve(size) ; 
  for (decltype(size) i=0 ; i<size ; ++i)
    v.push_back(Property::Ram(payload[2*i+0],payload[2*i+1])) ;
  return v ;
}

std::vector<Property::Ram> Property::Ram::      on_arm(Interface *iface) { return query(iface,0x10005) ; }
std::vector<Property::Ram> Property::Ram::on_videoCore(Interface *iface) { return query(iface,0x10006) ; }
