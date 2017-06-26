// BSD 2-Clause License, see github.com/ma16/rpio

#include "../Error.h"
#include "Message.h"
#include <cassert>
#include <sstream>
#include <stdexcept>

Rpi::Mbox::Message::Message(uint32_t tag,std::initializer_list<uint32_t> const &arg,uint32_t responseSize) 
  : tag(tag),
    arg(arg.begin(),arg.end()),
    requestSize(sizeof(uint32_t) * arg.size()),
    responseSize(responseSize),
    payloadSize(std::max(requestSize,responseSize))
{
  assert(arg.size() < 0xffffffff/sizeof(uint32_t)-3) ;
  // ...argument size in bytes; [future] make it a type
  assert(0 == (responseSize & (1u<<31))) ; 
  // ...MSB is used in the response to signal success; [future] make it a type
}

// Request : Size Code Payload End
//
// Size:uint32_t = Message size in bytes (including the Size field)
// Code:uint32_t = request code; always 0
// Payload       = Request-Property serialization list
// End:uint32_t  = end-tag, always 0
std::vector<uint32_t> Rpi::Mbox::Message::serialize_list(std::vector<uint32_t> const &payload)
{
  std::vector<uint32_t> buffer(payload.size() + 3,0) ;
  buffer[0] = sizeof(uint32_t) * (payload.size()+3) ;
  // [1] := code: always 0
  std::copy(payload.begin(),payload.end(),&buffer[2]) ;
  // [back] := end-tag: always 0
  return buffer ;
}

// Response : Size Code Payload End
//
// Size:uint32_t = same as in request
// Code:uint32_t : 0x80000000 # request successful
//               | 0x80000001 # error parsing request (partial response)
// Payload       = Response-Property serialization list
// End:uint32_t  = same as in request
std::vector<uint32_t> Rpi::Mbox::Message::deserialize_list(uint32_t const *buffer,uint32_t nbytes)
{
  if (buffer[0] != nbytes)
    throw Error("Mbox::Message:size mismatch") ;
  auto nwords = (nbytes + 3) / sizeof(uint32_t) ;
  if (nwords < 3) 
    throw Error("Mbox::Message:size too small") ;
  if (0 == (buffer[1] & (1u<<31)))
    throw Error("Mbox::Message::rejected by VideoCore") ;
  // we ignore if the response is marked as partial; it's up to
  // the message property deserialization to deal with that
  if (buffer[nwords-1] != 0)
    throw Error("Mbox::Message:end-tag mismatch") ; 
  return std::vector<uint32_t>(buffer+2,buffer+nwords-1) ;
}

// Request-Property : Tag Size Length Payload Padding
//
// Tag:uint32_t    = tag identifier
// Size:uint32_t   = Payload size in bytes
// Length:uint32_t = number of bytes within the Payload to cover request data
// Payload         = actual request data (gets overwritten by response data)
// Padding         = 0..3 bytes to make the payload a multiple of 32-bit
std::vector<uint32_t> Rpi::Mbox::Message::serialize_property() const 
{
  std::vector<uint32_t> buffer(3 + (this->payloadSize+3)/sizeof(uint32_t)) ;
  buffer[0] = this->tag ;
  buffer[1] = this->payloadSize ;
  buffer[2] = this->requestSize ;
  std::copy(this->arg.begin(),this->arg.end(),&buffer[3]) ;
  return buffer ;
}

// Response-Property : Tag Size Length Payload Padding
//
// Tag:uint32_t    = same as in request
// Size:uint32_t   = same as in request
// Length:uint32_t : bit[31]==0 # -> error
//                 | bit[31]==1 # -> bit[0..30] response size in bytes
// Payload         = actual response data (overwrites request data)
// Padding         = 0..3 bytes to make the payload a multiple of 32-bit
std::vector<uint32_t> Rpi::Mbox::Message::deserialize_property(std::vector<uint32_t> const &buffer,uint32_t *size_) const
{
  if (buffer[0] != this->tag) 
    throw Error("Mbox::Message:property-tag mismatch") ;
  if (buffer[1] != this->payloadSize) 
    throw Error("Mbox::Message:property-size mismatch") ;
  auto size = buffer[2] ;
  if (0 == (size & (1u << 31)))
    throw Error("Mbox::Message:property rejected by VideoCore") ; 
  size &= ~(1u << 31) ;
  // note that VideoCore may need more space than available in the
  // request. {size} reflects this required size. thus, if a client
  // can not predict the required size, it may send another request
  // that provides the required space.
  auto nbytes = std::min(size,this->payloadSize) ;
  // ...the actual response payload in this message
  auto nwords = (nbytes + 3) / sizeof(uint32_t) ;
  std::vector<uint32_t> payload(nwords) ;
  std::copy(&buffer[3],&buffer[3+nwords],&payload[0]) ;
  assert(size_ != nullptr) ;
  (*size_) = size ;
  return payload ;
}

std::vector<uint32_t> Rpi::Mbox::Message::serialize() const
{
  auto property = this->serialize_property() ;
  auto message = this->serialize_list(property) ;
  return message ;
}

std::vector<uint32_t> Rpi::Mbox::Message::deserialize(uint32_t const *buffer,uint32_t bufsiz,uint32_t *size_) const
{
  auto property = deserialize_list(buffer,bufsiz) ;
  uint32_t size ;
  auto payload = this->deserialize_property(property,&size) ;
  if (size_ != nullptr) {
    (*size_) = size ;
  }
  else if (size != this->responseSize) {
    throw Error("Mbox::Message:payload-size mismatch") ;
  }
  return payload ;
}
