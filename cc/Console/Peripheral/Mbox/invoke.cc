// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Mailbox / Property access
// --------------------------------------------------------------------

#include "../invoke.h"
#include <Neat/Numerator.h>
#include <Linux/PhysMem.h>
#include <Rpi/Mbox/Queue.h>
#include <Rpi/Mbox/Property.h>
#include <Ui/strto.h>
#include <iomanip>
#include <iostream>

static std::string toStr(std::array<uint8_t,6> mac)
{
  std::ostringstream os ;
  os << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(mac[0]) ;
  for (unsigned i=1 ; i<6 ; ++i)
    os << ':' << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(mac[i]) ;
  return os.str() ;
}

static void board(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  namespace Property = Rpi::Mbox::Property ;
  argL->finalize() ;
  std::cout << "MAC........" << toStr(Property::board_mac_address(iface)) << '\n'
	    << std::hex
	    << "Model......0x" << Property::board_model(iface) << '\n' 
	    << "Revision...0x" << Property::board_revision(iface) << '\n' 
	    << "Serial.....0x" << Property::board_serial(iface) << '\n' ;
}

static void clear(Rpi::Peripheral *rpi,Rpi::Mbox::Interface*,Ui::ArgL *argL)
{
  argL->finalize() ;
  Rpi::Mbox::Queue mbox(rpi) ;
  auto i = 0 ;
  while (mbox.readable()) {
    mbox.read() ;
    ++i ;
  }
  std::cout << "Number of pending responses: " << i << std::endl ;
}
  
static void clockRate(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  auto id = Ui::strto<uint32_t>(argL->pop()) ;
  auto rate = Ui::strto<uint32_t>(argL->pop()) ;
  auto skip = Ui::strto<uint32_t>(argL->pop()) ;
  argL->finalize() ;
  rate = Rpi::Mbox::Property::Clock::setRate(iface,id,rate,skip) ;
  std::cout << rate << std::endl ;
}

static void clockState(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  auto id = Ui::strto<uint32_t>(argL->pop()) ;
  auto state = Ui::strto(argL->pop(),Rpi::Mbox::Property::Clock::State()) ;
  argL->finalize() ;
  state = Rpi::Mbox::Property::Clock::setState(iface,id,state) ;
  std::cout << state.value() << std::endl ;
}

static void clockStatus(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  argL->finalize() ;
  using Clock = Rpi::Mbox::Property::Clock ;
  auto v = Clock::all(iface) ;
  std::cout << " # parent state         hz        min        max name\n"
	    << "-----------------------------------------------------\n" ;
  for (auto &m : v) {
    auto state = Clock::state(iface,m.id).value() ;
    auto  rate = Clock:: rate(iface,m.id) ;
    auto   min = Clock::  min(iface,m.id) ;
    auto   max = Clock::  max(iface,m.id) ;
    auto  name = Clock:: name(      m.id) ;
    std::cout << std::setw( 2) << m.id
	      << std::setw( 7) << m.parent
	      << std::setw( 6) << state
	      << std::setw(11) << rate 
	      << std::setw(11) << min 
	      << std::setw(11) << max 
	      << ' ' << name << '\n' ;
  }
}

static void clock(Rpi::Peripheral *rpi,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: [MODE]\n"
	      << '\n'
	      << "MODE : status              # print status information\n"
	      << "     | state ID STATE      # set clock state 0..3\n"
	      << "     | rate  ID RATE SKIP  # set clock rate\n"
	      << std::flush ;
    return ;
  }
  auto mode = argL->pop({"rate","state","status"}) ;
  switch (mode) {
  case 0: return   clockRate(rpi,iface,argL) ;
  case 1: return  clockState(rpi,iface,argL) ;
  case 2: return clockStatus(rpi,iface,argL) ;
  default: assert(0) ;
  }
}

static void command(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  namespace Property = Rpi::Mbox::Property ;
  argL->finalize() ;
  std::cout << Property::command_line(iface) << '\n' ;
}

static void devicePower(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  using Device = Rpi::Mbox::Property::Device ;
  using Num = Neat::Numerator<Device::Id,Device::Id::CCP2TX> ;
  auto id = Num::make(Ui::strto<Num::Domain>(argL->pop())) ;
  auto on = !argL->pop_if("off") ;
  argL->finalize() ;
  on = Rpi::Mbox::Property::Device::set_power_state(iface,id.e(),on,true) ;
  std::cout << on << std::endl ;
}

static void deviceStatus(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  argL->finalize() ;
  using Device = Rpi::Mbox::Property::Device ;
  using Num = Neat::Numerator<Device::Id,Device::Id::CCP2TX> ;
  std::cout << "# power timing name\n"
	    << "-------------------\n" ;
  auto i = Num() ; do {
    auto   name = Device::           name(      i.e()) ;
    auto  power = Device::get_power_state(iface,i.e()) ;
    auto timing = Device::         timing(iface,i.e()) ;
    std::cout << std::setw( 1) << i.n()
	      << std::setw( 6) << power
	      << std::setw( 7) << timing
	      << ' ' << name << '\n' ;
  }
  while (i.next()) ;
}

static void device(Rpi::Peripheral *rpi,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: [MODE]\n"
	      << '\n'
	      << "MODE : power ID [off]  # set power state\n"
	      << "     | status          # print status information\n"
	      << std::flush ;
    return ;
  }
  auto mode = argL->pop({"power","status"}) ;
  switch (mode) {
  case 0: return  devicePower(rpi,iface,argL) ;
  case 1: return deviceStatus(rpi,iface,argL) ;
  default: assert(0) ;
  }
}

static void dma(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  namespace Property = Rpi::Mbox::Property ;
  argL->finalize() ;
  auto mask = Property::dma_channels(iface) ;
  std::cout << "  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15\n"
	    << "------------------------------------------------\n" ;
  for (unsigned i=0 ; i<16 ; ++i)
    std::cout << ((mask & (1u<<i)) ? " ok" : "  -") ;
  std::cout << "\n" ;
}

static void firmware(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  namespace Property = Rpi::Mbox::Property ;
  argL->finalize() ;
  std::cout << "0x" << std::hex << Property::firmware_revision(iface) << '\n' ;
}

static void memory(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: [MODE]\n"
	      << '\n'
	      << "MODE : allocate SIZE ALIGN MODE  # return HANDLE to memory\n"
	      << "     | count    [-r] [-s]        # count number of allocated blocks\n"
	      << "     | lock     HANDLE           # lock memory and return address\n"
	      << "     | release  HANDLE           # free memory\n"
	      << "     | unlock   HANDLE           # unlock memory, don't free\n"
	      << '\n'
	      << "memory allocation flags (MODE)\n"
	      << "  bit 0: discardable\n"
	      << "  bit 2: direct\n"
	      << "  bit 3: coherent\n"
	      << "  bit 4: zero-init\n"
	      << "  bit 5: no-init\n"
	      << "  bit 6: perma-lock\n"
	      << std::flush ;
    return ;
  }
  using Memory = Rpi::Mbox::Property::Memory ;
  auto command = argL->pop({"allocate","count","lock","release","unlock"}) ;
  if (command >= 2) {
    auto handle = Ui::strto<uint32_t>(argL->pop()) ;
    argL->finalize() ;
    switch (command) {
    case 2: std::cout << std::hex << "0x" << Memory::lock(iface,handle) << '\n' ; break ;
    case 3: std::cout << (Memory::release(iface,handle) ? "ok" : "error") << '\n' ; break ;
    case 4: std::cout << (Memory:: unlock(iface,handle) ? "ok" : "error") << '\n' ; break ;
    default: assert(0) ;
    }
    return ;
  }
  if (command == 0)
  {
      auto      size = Ui::strto<uint32_t>(argL->pop()) ;
      auto alignment = Ui::strto<uint32_t>(argL->pop()) ;
      auto      mode = Memory::Mode::deserialize(Ui::strto<uint32_t>(argL->pop())) ;
      argL->finalize() ;
      std::cout << std::hex << "0x" << Memory::allocate(iface,size,alignment,mode) << '\n' ;
      return ;
  }
  auto release = argL->pop_if("-r") ;
  auto show = argL->pop_if("-s") ;
  argL->finalize() ;
  auto n = 0u ;
  for (auto i=0u ; i<0x1000 ; ++i)
      if (0 != Memory::lock(iface,i))
      {
	  ++n ;
	  if (show)
	    std::cout << std::dec << i << ' '
		      << std::hex << "0x"
		      << Memory::lock(iface,i) << '\n' ;
	  if (release)
	      Memory::release(iface,i) ;
      }
  std::cout << n << std::endl ;
}
  
static void ram(Rpi::Peripheral*,Rpi::Mbox::Interface *iface,Ui::ArgL *argL)
{
  argL->finalize() ;
  auto v = Rpi::Mbox::Property::Ram::on_arm(iface) ;
  std::cout << std::hex ;
  std::cout << "ARM:\n" ;
  for (auto &m : v)
    std::cout << "base:0x" << m.base << " size:0x" << m.size << '\n' ;
  v = Rpi::Mbox::Property::Ram::on_videoCore(iface) ;
  std::cout << "VideoCore:\n" ;
  for (auto &m : v)
    std::cout << "base:0x" << m.base << " size:0x" << m.size << '\n' ;
}
  
Rpi::Mbox::Interface::shared_ptr make_if(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->pop_if("-p")) {
    auto e = Ui::strto(argL->pop(),Rpi::Bus::Coherency::Enum()) ;
    return Rpi::Mbox::Interface::make(Rpi::Mbox::Queue(rpi),e) ;
  }
  auto vcio = Rpi::Mbox::Vcio::shared_ptr(new Rpi::Mbox::Vcio) ;
  return Rpi::Mbox::Interface::make(vcio) ;
}

void Console::Peripheral::Mbox::
invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: [-p CO] MODE\n"
	      << '\n'
	      << "-p CO: optional peripheral Mailbox access with coherency 0..3\n"
	      << "default: via ioctl on /dev/vcio\n"
	      << '\n'
	      << "MODE : board      # print board information\n"
	      << "     | clear      # clear peripheral Maxilbox queue\n"
	      << "     | command    # print startup command line string\n"
	      << "     | clock...   # manage peripheral clocks\n"
	      << "     | dma        # show available DMA channels\n"
	      << "     | device...  # manage peripheral device\n"
	      << "     | firmware   # show firmware number\n"
	      << "     | memory...  # manage VideoCore memory\n"
	      << "     | ram        # show RAM information\n"
	      << std::flush ;
    return ;
  }
  auto iface = make_if(rpi,argL) ;
  std::string arg = argL->pop() ;
  if      (arg ==    "board")    board(rpi,iface.get(),argL) ;
  else if (arg ==    "clear")    clear(rpi,iface.get(),argL) ;
  else if (arg ==  "command")  command(rpi,iface.get(),argL) ;
  else if (arg ==    "clock")    clock(rpi,iface.get(),argL) ;
  else if (arg ==      "dma")      dma(rpi,iface.get(),argL) ;
  else if (arg ==   "device")   device(rpi,iface.get(),argL) ;
  else if (arg == "firmware") firmware(rpi,iface.get(),argL) ;
  else if (arg ==   "memory")   memory(rpi,iface.get(),argL) ;
  else if (arg ==      "ram")      ram(rpi,iface.get(),argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}
