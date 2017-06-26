// BSD 2-Clause License, see github.com/ma16/rpio

#include "Interface.h"
#include <Linux/PhysMem.h>
#include <cassert>
#include <cstring>

#define DEBUG 0

#if DEBUG
#include <iomanip>
#include <iostream>
static std::string toString(uint32_t const *buffer) 
{
  std::ostringstream os ;
  auto n = buffer[0] ;
  os << std::hex << n ;
  for (auto i=1u ; i<(n+3)/4 ; ++i)
    os << ' ' << std::hex << std::setw(8) << buffer[i] ;
  return os.str() ;
}
#endif

struct Nif : Rpi::Mbox::Interface // native (peripheral) interface
{
  virtual std::vector<uint32_t> query(Rpi::Mbox::Message const &message,uint32_t *size) override
  {
    auto xfer = message.serialize() ;
    auto xfer_size = xfer[0] ;
    assert(xfer_size <= 0x1000) ;
    // ...if more than one page, the page numbers must be subsequent
    auto mem = Linux::PhysMem::allocate(0x1000) ;
    memcpy(mem->virt(),&xfer[0],xfer_size) ;
#if DEBUG    
    std::cerr << "send: " << toString(mem->as<uint32_t*>()) << std::endl ;
#endif
    auto addr = mem->phys(0).first ;
    assert(0 == (addr & 0xf)) ;
    // ...must be 16-byte aligned
    addr |= 0x8 ;
    // ...the "channel" number for the property interface
    addr |= this->co.prefix() ;
    queue.query(addr) ;
#if DEBUG    
    std::cerr << "recv: " << toString(mem->as<uint32_t*>()) << std::endl ;
#endif
    memcpy(&xfer[0],mem->virt(),xfer_size) ; 
    auto payload = message.deserialize(&xfer[0],xfer_size,size) ;
    return payload ;
  }

  Rpi::Mbox::Queue queue ; Rpi::Bus::Coherency co ;

  Nif(Rpi::Mbox::Queue queue,Rpi::Bus::Coherency co) : queue(queue),co(co) {}
} ;

Rpi::Mbox::Interface::shared_ptr Rpi::Mbox::Interface::make(Queue queue,Rpi::Bus::Coherency co)
{
  return std::shared_ptr<Nif>(new Nif(queue,co)) ;
}

struct Lif : Rpi::Mbox::Interface // Linux /dev/vcio ioctl inetrface
{
  virtual std::vector<uint32_t> query(Rpi::Mbox::Message const &message,uint32_t *size) override
  {
    auto xfer = message.serialize() ;
    auto xfer_size = xfer[0] ;
    vcio->query(&xfer[0]) ;
    auto payload = message.deserialize(&xfer[0],xfer_size,size) ;
    return payload ;
  }

  Rpi::Mbox::Vcio::shared_ptr vcio ; Lif(Rpi::Mbox::Vcio::shared_ptr vcio) : vcio(vcio) {}
} ;

Rpi::Mbox::Interface::shared_ptr Rpi::Mbox::Interface::make(Rpi::Mbox::Vcio::shared_ptr vcio)
{
  return std::shared_ptr<Lif>(new Lif(vcio)) ;
}
