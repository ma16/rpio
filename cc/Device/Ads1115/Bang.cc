// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <chrono>

static auto const Lo = Rpi::Gpio::Output::Lo ;
static auto const Hi = Rpi::Gpio::Output::Hi ;

Device::Ads1115::Bang::Bang(
    Rpi::Peripheral *rpi,
    Rpi::Pin sclPin,
    Rpi::Pin sdaPin,
    Addr addr,
    Timing<uint32_t> const &timing,
    bool monitor) 
    : rpi(rpi),counter(rpi),gpio(rpi),sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing),monitor(monitor)
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

void Device::Ads1115::Bang::sclHi()
{
    assert(this->gpio.getMode(this->sclPin) == Rpi::Gpio::Mode::Out) ;
    this->gpio.setOutput<Hi>(this->sclPin) ; 
}

void Device::Ads1115::Bang::sclLo()
{
    assert(this->gpio.getMode(this->sclPin) == Rpi::Gpio::Mode::Out) ;
    this->gpio.setOutput<Lo>(this->sclPin) ; 
}

void Device::Ads1115::Bang::sdaLo()
{
    this->gpio.setMode(this->sdaPin,Rpi::Gpio::Mode::Out) ;
}

void Device::Ads1115::Bang::sdaOff()
{
    this->gpio.setMode(this->sdaPin,Rpi::Gpio::Mode::In) ;
}

bool Device::Ads1115::Bang::sdaLevel()
{
    assert(this->gpio.getMode(this->sdaPin) == Rpi::Gpio::Mode::In) ;
    return this->gpio.getLevel(this->sdaPin) ;
}

// --------------------------------------------------------------------

void Device::Ads1115::Bang::start()
{
    // (Hi,Off) -> (Lo,Off)
    this->hold(this->timing.buf) ;
    this->sdaLo() ;
    this->hold(this->timing.hdsta) ;
    this->sclLo() ;
    this->sdaOff() ; // [todo] we're going to send data, so keep it Lo
}

void Device::Ads1115::Bang::stop()
{
    // (Lo,Off) -> (Hi,Off)
    this->sdaLo() ; 
    this->hold(this->timing.low) ;
    this->sclHi() ;
    this->hold(this->timing.susto) ;
    this->sdaOff() ; 
}

// --------------------------------------------------------------------

void Device::Ads1115::Bang::sendBit(bool hi)
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

void Device::Ads1115::Bang::sendByte(uint8_t byte)
{
    // (Lo,Off) -> (Lo,Off)
    auto data = static_cast<int>(byte) ;
    for (auto i=0 ; i<8 ; ++i) {
	this->sendBit(0 != (data & 0x80)) ;
	data <<= 1 ;
    }
}

bool Device::Ads1115::Bang::recvBit()
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

uint8_t Device::Ads1115::Bang::recvByte()
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

unsigned Device::Ads1115::Bang::hold(unsigned delay)
{
    auto t0 = this->counter.clock() ;
    // [note] on a slow clock (which may return the same value several
    // times) we'd need to loop to the next point in time (i.e. VC Timer).
    auto t1 = this->counter.clock() ;
    while (t1-t0 < delay) 
	t1 = this->counter.clock() ;
    return t1 ;
}

// --------------------------------------------------------------------

bool Device::Ads1115::Bang::doReset()
{
    this->start() ;
    // general address (000:0000,0)B
    this->sendByte(0) ;
    auto success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    // reset command
    this->sendByte(6) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    this->stop() ;
    return true ;
}

boost::optional<uint16_t> Device::Ads1115::Bang::read(uint8_t rix)
{
    this->start() ;
    // address + WRITE
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(byte) ;
    auto success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    // point to register to read
    this->sendByte(rix) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    this->stop() ;
    this->start() ;
    // address + READ
    byte = static_cast<uint8_t>((this->addr.value()<<1) | 1) ;
    this->sendByte(byte) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    // actual register read
    auto hi = this->recvByte() ;
    this->sendAck() ;
    auto lo = this->recvByte() ;
    this->sendAck() ;
    this->stop() ;
    return static_cast<uint16_t>((hi<<8) | lo) ;
}

bool Device::Ads1115::Bang::write(uint8_t rix,uint16_t data)
{
    this->start() ;
    // address + WRITE
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(byte) ;
    auto success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    // point to register to write
    this->sendByte(rix) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    // write config register
    this->sendByte(static_cast<uint8_t>(data>>8)) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    this->sendByte(static_cast<uint8_t>(data)) ;
    success = this->recvAck() ;
    if (!success)
	return set(Error::NoAck) ;
    this->stop() ;
    return true ;
}

// --------------------------------------------------------------------

void Device::Ads1115::Bang::sendByte(
    RpiExt::Bang::Enqueue *q,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack)
{
    // (scl,sda) == (low,off)
    auto mask = 1u << 7 ;
    do
    {
	q->time(t0) ;
	auto bit = (byte & mask) ? Line::Off : Line::Low ;
	if (bit != sda)
	{
	    q->sleep(this->timing.hddat) ; // still holding old SDA
	    if (bit == Line::Low) q->low(this->sdaPin) ;
	    else                  q->off(this->sdaPin) ;
	    q->sleep(this->timing.sudat) ; // setting up new SDA
	    sda = bit ;
	}
	q->wait(t0,this->timing.low) ;
	q->off(this->sclPin) ; // todo: rising edge must be <= t_rise
	q->sleep(this->timing.high) ;
	q->low(this->sclPin) ; // todo: falling edge must be <= t_fall
	mask >>= 1 ;
    }
    while (mask != 0) ;

    q->time(t0) ;
    if (sda != Line::Off)
	q->off(this->sdaPin) ;
    q->wait(t0,this->timing.low) ;
    q->off(this->sclPin) ; 
    q->time(t0) ;
    q->wait(t0,this->timing.sudat) ; // ??
    q->levels(ack) ;
    q->wait(t0,this->timing.high) ; 
    q->low(this->sclPin) ; 
    // (scl,sda) == (lo,off)
}

Device::Ads1115::Bang::Script Device::Ads1115::Bang::
makeScript(uint16_t word,Record *record)
{
    // (scl,sda) == (off,off)
    RpiExt::Bang::Enqueue q ;
    // start 
    q.sleep(this->timing.buf) ;
    q.low(this->sdaPin) ;
    q.sleep(this->timing.hdsta) ;
    q.low(this->sclPin) ;
    // -> (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(&q,Line::Low,byte,&record->t0,&record->ack[0]) ;
    // -> register pointer (config register)
    this->sendByte(&q,Line::Off,1,&record->t0,&record->ack[1]) ;
    // -> 16-bit register
    byte = static_cast<uint8_t>(word>>8) ;
    this->sendByte(&q,Line::Off,byte,&record->t0,&record->ack[2]) ;
    byte = static_cast<uint8_t>(word) ;
    this->sendByte(&q,Line::Off,byte,&record->t0,&record->ack[3]) ;
    // stop: (scl,sda) == (low,off)
    q.time(&record->t0) ;
    q.low(this->sdaPin) ;
    q.sleep(this->timing.sudat) ; 
    q.sleep(this->timing.low) ;
    q.off(this->sclPin) ;
    q.sleep(this->timing.susto) ;
    q.off(this->sdaPin) ;
    return q.vector() ;
    // (scl,sda) == (off,off)
}

// --------------------------------------------------------------------

bool /* todo: error code */ Device::Ads1115::Bang::writeConfig2(uint16_t word)
{
    Record record ;

    auto q = this->makeScript(word,&record) ;
    
    RpiExt::Bang scheduler(this->rpi) ;

    scheduler.execute(q.begin(),q.end()) ;

    return true ;
}
