// BSD 2-Clause License, see github.com/ma16/rpio

#include "base.h"
#include "../Message.h"

namespace Property = Rpi::Mbox::Property ;

uint32_t Property::firmware_revision(Interface *iface) 
{
  static Message const message({0x1,{},4}) ;
  return iface->query(message,nullptr).front() ;
}

uint32_t Property::board_model(Interface *iface) 
{
  static Message const message({0x10001,{},4}) ;
  return iface->query(message,nullptr).front() ;
}

uint32_t Property::board_revision(Interface *iface) 
{
  static Message const message({0x10002,{},4}) ;
  return iface->query(message,nullptr).front() ;
}

std::array<uint8_t,6> Property::board_mac_address(Interface *iface) 
{
  static Message const message({0x10003,{},6}) ;
  auto payload = iface->query(message,nullptr) ;
  auto src = reinterpret_cast<uint8_t*>(&payload.front()) ;
  std::array<uint8_t,6> mac ;
  std::copy(src+0,src+6,&mac[0]) ;
  return mac ;
}

uint64_t Property::board_serial(Interface *iface) 
{
  static Message const message({0x10004,{},8}) ;
  auto payload = iface->query(message,nullptr) ;
  auto src = reinterpret_cast<uint64_t*>(&payload.front()) ;
  return src[0] ;
}

std::string Property::command_line(Interface *iface)
{
  Message const probe({0x50001,{},0}) ;
  uint32_t nbytes ;
  iface->query(probe,&nbytes) ;
  Message const message({0x50001,{},nbytes}) ;
  auto payload = iface->query(message,nullptr) ;
  auto p = reinterpret_cast<char*>(&payload.front()) ;
  return std::string(p,nbytes) ;
}

uint32_t Property::dma_channels(Interface *iface) 
{
  Message const message({0x60001,{},4}) ;
  return iface->query(message,nullptr).front() ;
}
