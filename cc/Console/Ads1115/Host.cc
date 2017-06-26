// BSD 2-Clause License, see github.com/ma16/rpio

#include "Host.h"
#include <chrono>

#include <iostream> // debug
#define NDEBUG 1 // debug

static auto const Lo = Rpi::Gpio::Output::Lo ;
static auto const Hi = Rpi::Gpio::Output::Hi ;

Console::Ads1115::Host::Host(Rpi::Peripheral *rpi,
			  Rpi::Pin sclPin,
			  Rpi::Pin sdaPin,
			  addr_t addr,
			  Timing const &timing,
			  bool monitor) 
  : counter(rpi),gpio(rpi),sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing),monitor(monitor)
{
  // init
  this->gpio.setOutput<Hi>(this->sclPin) ;
  this->gpio.setMode(this->sclPin,Rpi::Gpio::Mode::Out) ;
  this->gpio.setMode(this->sdaPin,Rpi::Gpio::Mode::In) ;
  this->gpio.setOutput<Lo>(this->sdaPin) ;
  // (Hi,Off)
  
  // make sure the ARM counter is on and runs at 100 Mhz
  if (!this->counter.enabled())
    throw std::runtime_error("please enable ARM counter") ;
  if (this->counter.prescaler() != 3)
    throw std::runtime_error("please set ARM prescaler to <3>") ;
}

// --------------------------------------------------------------------

void Console::Ads1115::Host::sclHi()
{
  assert(this->gpio.getMode(this->sclPin) == Rpi::Gpio::Mode::Out) ;
  this->gpio.setOutput<Hi>(this->sclPin) ; 
}

void Console::Ads1115::Host::sclLo()
{
  assert(this->gpio.getMode(this->sclPin) == Rpi::Gpio::Mode::Out) ;
  this->gpio.setOutput<Lo>(this->sclPin) ; 
}

void Console::Ads1115::Host::sdaLo()
{
  this->gpio.setMode(this->sdaPin,Rpi::Gpio::Mode::Out) ;
}

void Console::Ads1115::Host::sdaOff()
{
  this->gpio.setMode(this->sdaPin,Rpi::Gpio::Mode::In) ;
}

bool Console::Ads1115::Host::sdaLevel()
{
  assert(this->gpio.getMode(this->sdaPin) == Rpi::Gpio::Mode::In) ;
  return this->gpio.getLevel(this->sdaPin) ;
}

// --------------------------------------------------------------------

void Console::Ads1115::Host::start()
{
  // (Hi,Off) -> (Lo,Off)
  this->hold(this->timing.buf) ;
  this->sdaLo() ;
  this->hold(this->timing.hdsta) ;
  this->sclLo() ;
  this->sdaOff() ; // [todo] we're going to send data, so keep it Lo
}

void Console::Ads1115::Host::stop()
{
  // (Lo,Off) -> (Hi,Off)
  this->sdaLo() ; 
  this->hold(this->timing.low) ;
  this->sclHi() ;
  this->hold(this->timing.susto) ;
  this->sdaOff() ; 
}

// --------------------------------------------------------------------

void Console::Ads1115::Host::sendBit(bool hi)
{
  // (Lo,Off) -> (Lo,Off)
  if (!hi)
    this->sdaLo() ;
  this->hold(std::max(this->timing.low,this->timing.sudat)) ; // [todo] better: get t0 when SCL=HL; then wait(max(low-t0,sudat))
  this->sclHi() ;
  this->hold(this->timing.high) ;
  this->sclLo() ;
  if (!hi)
    this->sdaOff() ;
}

void Console::Ads1115::Host::sendByte(uint8_t byte)
{
  // (Lo,Off) -> (Lo,Off)
  auto data = static_cast<int>(byte) ;
  for (auto i=0 ; i<8 ; ++i) {
    this->sendBit(0 != (data & 0x80)) ;
    data <<= 1 ;
  }
}

bool Console::Ads1115::Host::recvBit()
{
  // (Lo,Off) -> (Lo,Off)
  this->hold(this->timing.low) ;
  this->sclHi() ;
  this->hold(this->timing.sudat) ; // [todo]
  auto level = this->sdaLevel() ;
  this->hold(this->timing.high) ; // [todo]
  this->sclLo() ;
  return level ;
}

uint8_t Console::Ads1115::Host::recvByte()
{
  // (Lo,Off) -> (Lo,Off)
  auto data = 0u ;
  for (auto i=0 ; i<8 ; ++i) {
    data <<= 1 ;
    data |= this->recvBit() ;
  }
  return static_cast<uint8_t>(data) ;
}

// --------------------------------------------------------------------

unsigned Console::Ads1115::Host::hold(unsigned delay)
{
  auto t0 = this->counter.clock() ;
  // [note] on a slow clock (which may return the same value several
  // times) we'd need to loop to the next point in time (i.e. VC Timer).
  auto t1 = this->counter.clock() ;
  while (t1-t0 < delay/10) 
    t1 = this->counter.clock() ;
  return t1 ;
}

// --------------------------------------------------------------------

bool Console::Ads1115::Host::doReset()
{
  this->start() ;
  // general address (000:0000,0)B
  this->sendByte(0) ;
  auto success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  // reset command
  this->sendByte(6) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  this->stop() ;
  return true ;
}

boost::optional<uint16_t> Console::Ads1115::Host::read(uint8_t rix)
{
  this->start() ;
  // address + WRITE
  auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
  this->sendByte(byte) ;
  auto success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  // point to register to read
  this->sendByte(rix) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  this->stop() ;
  this->start() ;
  // address + READ
  byte = static_cast<uint8_t>((this->addr.value()<<1) | 1) ;
  this->sendByte(byte) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  // actual register read
  auto hi = this->recvByte() ;
  this->sendAck() ;
  auto lo = this->recvByte() ;
  this->sendAck() ;
  this->stop() ;
  return static_cast<uint16_t>((hi<<8) | lo) ;
}

bool Console::Ads1115::Host::write(uint8_t rix,uint16_t data)
{
  this->start() ;
  // address + WRITE
  auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
  this->sendByte(byte) ;
  auto success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  // point to register to write
  this->sendByte(rix) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  // write config register
  this->sendByte(static_cast<uint8_t>(data>>8)) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  this->sendByte(static_cast<uint8_t>(data)) ;
  success = this->recvAck() ;
  if (!success)
    return set(error_t::NoAck) ;
  this->stop() ;
  return true ;
}

