// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Message_h_
#define _Rpi_Mbox_Message_h_

// see github.com/raspberrypi/firmware/wiki/Mbox-property-interface

#include <cstdint>
#include <vector>

namespace Rpi { namespace Mbox { struct Message
{
  Message(uint32_t tag,std::initializer_list<uint32_t> const&,uint32_t rspSize) ;

  std::vector<uint32_t> serialize() const ;
  // --the address of such serialized data is passed to Mbox queue,
  //   though addresses passed to Mailbox must be physical addresses
  // --the native Mailbox interface requires a 16-byte aligned address
  // --the serialized data (which contains space for the response)
  //   is then overwritten by the VideoCore

  std::vector<uint32_t> deserialize(uint32_t const *buffer,uint32_t bufsiz,uint32_t *size=nullptr) const ;
  // --returns the response's payload
  // --throws if the response is invalid or doesn't match the request
  // --the VideoCore may have requested more space for the response
  //   (indicated by payload.size() < (*size))
  // --throws if size==nullptr && if the response's payload size doesn't
  //   match the response size provided in the constructor

private:

  uint32_t              tag ; 
  std::vector<uint32_t> arg ; 
  uint32_t      requestSize ;
  uint32_t     responseSize ;
  uint32_t      payloadSize ;

  static std::vector<uint32_t> serialize_list(std::vector<uint32_t> const &payload) ;
  // ...serializes Mailbox request with serveral properties
  
  static std::vector<uint32_t> deserialize_list(uint32_t const *buffer,uint32_t nbytes) ;
  // ...deserializes Mailbox response with serveral properties
  
  std::vector<uint32_t> serialize_property() const ;
  // ...serializes a single property

  std::vector<uint32_t> deserialize_property(std::vector<uint32_t> const&,uint32_t *size) const ;
  // ...deserializes a single property

} ; /* Message */ } /* Mbox */ } /* Rpi */

#endif // _Rpi_Mbox_Message_h_
