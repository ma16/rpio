// BSD 2-Clause License, see github.com/ma16/rpio

#include "Property.h"
#include <cassert>
#include <cstring> // strcasecmp

uint32_t Rpi::Mbox::PropertyZ::firmware_revision() 
{
  static Message const request({0x1,{},4}) ;
  return this->interface->query(request,nullptr).front() ;
}

uint32_t Rpi::Mbox::PropertyZ::board_model() 
{
  static Message const request({0x10001,{},4}) ;
  return this->interface->query(request,nullptr).front() ;
}

uint32_t Rpi::Mbox::PropertyZ::board_revision() 
{
  static Message const request({0x10002,{},4}) ;
  return this->interface->query(request,nullptr).front() ;
}

std::array<uint8_t,6> Rpi::Mbox::PropertyZ::board_mac_address() 
{
  static Message const request({0x10003,{},6}) ;
  auto data = this->interface->query(request,nullptr) ;
  auto src = reinterpret_cast<uint8_t*>(&data.front()) ;
  std::array<uint8_t,6> mac ;
  std::copy(src+0,src+6,&mac[0]) ;
  return mac ;
}

uint64_t Rpi::Mbox::PropertyZ::board_serial() 
{
  static Message const request({0x10004,{},8}) ;
  auto data = this->interface->query(request,nullptr) ;
  auto src = reinterpret_cast<uint64_t*>(&data.front()) ;
  return src[0] ;
}

std::string Rpi::Mbox::PropertyZ::command_line()
{
  auto tag = 0x50001u ;
  Message const probe({tag,{},0}) ;
  uint32_t nbytes ;
  this->interface->query(probe,&nbytes) ;
  // [todo] nbytes may have to fit into a single page
  Message const request({0x50001,{},nbytes}) ;
  auto data = this->interface->query(request,nullptr) ;
  auto p = reinterpret_cast<char*>(&data.front()) ;
  return std::string(p,nbytes) ;
}

uint32_t Rpi::Mbox::PropertyZ::dma_channels() 
{
  Message const request({0x60001,{},4}) ;
  return this->interface->query(request,nullptr).front() ;
}
