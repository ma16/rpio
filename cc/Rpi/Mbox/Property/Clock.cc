// BSD 2-Clause License, see github.com/ma16/rpio

#include "Clock.h"
#include "../Message.h"
#include "../../Error.h"

using Clock = Rpi::Mbox::Property::Clock ;

std::string Clock::name(uint32_t id)
{
  static char const* const array[] = { "","EMMC","UART","ARM","CORE","V3D","H264","ISP","SDRAM","PIXEL","PWM" } ;
  static_assert(11 == sizeof(array)/sizeof(array[0]),"") ;
  if (id > 10)
    id = 0 ;
  // ...hence valid id values are: 1..10
  return array[id] ;
}
  
std::vector<Clock> Clock::all(Interface *iface) 
{
  static Message const probe({0x10007,{},0}) ;
  uint32_t nbytes ;
  iface->query(probe,&nbytes) ;
  auto size = nbytes / 8 ;
  if (8u*size != nbytes)
    throw Error("Mbox::Property::Clock:unexpected size") ;
  Message const message({0x10007,{},nbytes}) ;
  auto payload = iface->query(message,nullptr) ;
  std::vector<Clock> v ; v.reserve(size) ;
  for (decltype(size) i=0 ; i<size ; ++i)
    v.push_back(Clock(payload[2*i+0],payload[2*i+1])) ;
  return v ;
} 

static uint32_t query(Rpi::Mbox::Interface *iface,uint32_t tag,std::initializer_list<uint32_t> const& args)
{
  Rpi::Mbox::Message m(tag,args,8u) ;
  auto payload = iface->query(m,nullptr) ;
  if (payload[0] != *(args.begin()))
    throw Rpi::Error("Mbox::Property::Clock:id does not match") ;
  return payload[1] ;
}

Clock::State Clock::   state(Interface *iface,uint32_t id)                             { return State::make(query(iface,0x30001,{id,             })) ; }
Clock::State Clock::setState(Interface *iface,uint32_t id,State state)                 { return State::make(query(iface,0x38001,{id,state.value()})) ; }
uint32_t     Clock::    rate(Interface *iface,uint32_t id)                             { return             query(iface,0x30002,{id,             })  ; }
uint32_t     Clock:: setRate(Interface *iface,uint32_t id,uint32_t rate,uint32_t skip) { return             query(iface,0x38002,{id,rate,skip    })  ; }
uint32_t     Clock::     max(Interface *iface,uint32_t id)                             { return             query(iface,0x30004,{id,             })  ; }
uint32_t     Clock::     min(Interface *iface,uint32_t id)                             { return             query(iface,0x30007,{id,             })  ; }

// [note] documentation says: "a rate of 0 is returned if the clock does not exist."
//   however it appears that the device may exist despite of a zero-value

