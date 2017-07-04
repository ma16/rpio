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
    Timing<uint32_t> const &timing)
    : rpi(rpi),counter(rpi),gpio(rpi),sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing)
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

void Device::Ads1115::Bang::start(RpiExt::Bang::Enqueue *q)
{
    // (scl,sda) == (off,off)
    q->sleep(this->timing.buf) ;
    q->low(this->sdaPin) ;
    q->sleep(this->timing.hdsta) ;
    q->low(this->sclPin) ;
    // (scl,sda) == (low,low)
}

void Device::Ads1115::Bang::stop(RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0)
{
    // (scl,sda) == (fall,*)
    q->time(t0) ;
    if (sda != Line::Low)
	q->low(this->sdaPin) ;
    q->sleep(this->timing.sudat) ; 
    q->wait(t0,this->timing.low) ;
    q->off(this->sclPin) ;
    q->sleep(this->timing.susto) ;
    q->off(this->sdaPin) ;
    // (scl,sda) == (off,off)
}

// --------------------------------------------------------------------

void Device::Ads1115::Bang::sendBit(RpiExt::Bang::Enqueue *q,Line from,Line to,uint32_t *t0)
{
    // (scl,sda) == (low,*)
    q->time(t0) ;
    if (from != to)
    {
	q->sleep(this->timing.hddat) ; // still holding old SDA
	if (to == Line::Low) q->low(this->sdaPin) ;
	else                 q->off(this->sdaPin) ;
	q->sleep(this->timing.sudat) ; // setting up new SDA
    }
    q->wait(t0,this->timing.low) ;
    q->off(this->sclPin) ; // todo: rising edge must be <= t_rise
    q->sleep(this->timing.high) ;
    q->low(this->sclPin) ; // todo: falling edge must be <= t_fall
    // (scl,sda) == (low,*)
}

void Device::Ads1115::Bang::recvBit(RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0,uint32_t *levels)
{
    // (scl,sda) == (low,*)
    q->time(t0) ;
    if (sda != Line::Off)
	q->off(this->sdaPin) ;
    q->sleep(this->timing.low) ;
    q->off(this->sclPin) ; 
    q->time(t0) ;
    q->wait(t0,this->timing.sudat) ; // ??
    q->levels(levels) ;
    q->wait(t0,this->timing.high) ; 
    q->low(this->sclPin) ; 
    // (scl,sda) == (low,off)
}

// --------------------------------------------------------------------

void Device::Ads1115::Bang::sendByte(
    RpiExt::Bang::Enqueue *q,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack)
{
    // (scl,sda) == (low,off)
    auto mask = 1u << 7 ;
    do
    {
	auto bit = (byte & mask) ? Line::Off : Line::Low ;
	this->sendBit(q,sda,bit,t0) ;
	sda = bit ;
	mask >>= 1 ;
    }
    while (mask != 0) ;
    this->recvBit(q,sda,t0,ack) ;
    // (scl,sda) == (lo,off)
}

void Device::Ads1115::Bang::recvByte(
    RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0,Record::Read::Byte *byte)
{
    // (scl,sda) == (low,*)
    this->recvBit(q,sda,t0,&byte->at(0)) ;
    for (size_t i=1 ; i<8 ; ++i)
	this->recvBit(q,Line::Off,t0,&byte->at(i)) ;
    this->sendBit(q,Line::Off,Line::Low,t0) ;
    // (scl,sda) == (lo,lo)
}

// --------------------------------------------------------------------

Device::Ads1115::Bang::Script Device::Ads1115::Bang::
scriptReset(Record::Reset *record)
{
    // (scl,sda) == (off,off)
    RpiExt::Bang::Enqueue q ;
    this->start(&q) ;
    // general address (000:0000,0)B
    this->sendByte(&q,Line::Low,0,&record->t0,&record->ackA.at(0)) ;
    // reset command: 0x6
    this->sendByte(&q,Line::Off,6,&record->t0,&record->ackA.at(1)) ;
    this->stop(&q,Line::Off,&record->t0) ;
    return q.vector() ;
    // (scl,sda) == (off,off)
}

Device::Ads1115::Bang::Script Device::Ads1115::Bang::
scriptRead(uint8_t rix,Record::Read *record)
{
    // (scl,sda) == (off,off)
    RpiExt::Bang::Enqueue q ;
    this->start(&q) ;
    // -> (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(&q,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register pointer (config register)
    this->sendByte(&q,Line::Off,rix,&record->t0,&record->ackA.at(1)) ;
    this->stop(&q,Line::Off,&record->t0) ;
    this->start(&q) ;
    // -> (address + read-bit)
    byte = static_cast<uint8_t>((this->addr.value()<<1) | 1) ;
    this->sendByte(&q,Line::Low,byte,&record->t0,&record->ackA.at(2)) ;
    // <- 16-bit register
    this->recvByte(&q,Line::Off,&record->t0,&record->byteA.at(0)) ;
    this->recvByte(&q,Line::Low,&record->t0,&record->byteA.at(1)) ;
    this->stop(&q,Line::Low,&record->t0) ;
    return q.vector() ;
    // (scl,sda) == (off,off)
}

Device::Ads1115::Bang::Script Device::Ads1115::Bang::
scriptWrite(uint16_t word,Record::Write *record)
{
    // (scl,sda) == (off,off)
    RpiExt::Bang::Enqueue q ;
    this->start(&q) ;
    // -> (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(&q,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register pointer (config register)
    this->sendByte(&q,Line::Off,1,&record->t0,&record->ackA.at(1)) ;
    // -> 16-bit register
    byte = static_cast<uint8_t>(word>>8) ;
    this->sendByte(&q,Line::Off,byte,&record->t0,&record->ackA.at(2)) ;
    byte = static_cast<uint8_t>(word) ;
    this->sendByte(&q,Line::Off,byte,&record->t0,&record->ackA.at(3)) ;
    this->stop(&q,Line::Off,&record->t0) ;
    return q.vector() ;
    // (scl,sda) == (off,off)
}

// --------------------------------------------------------------------

Device::Ads1115::Bang::Record::Reset Device::Ads1115::Bang::doReset()
{
    Record::Reset record ;
    auto q = this->scriptReset(&record) ;
    RpiExt::Bang scheduler(this->rpi) ;
    scheduler.execute(q.begin(),q.end()) ;
    return record ;
}

Device::Ads1115::Bang::Record::Read Device::Ads1115::Bang::readConfig()
{
    Record::Read record ;

    auto q = this->scriptRead(1,&record) ;
    
    RpiExt::Bang scheduler(this->rpi) ;

    scheduler.execute(q.begin(),q.end()) ;

    return record ;
}

Device::Ads1115::Bang::Record::Read Device::Ads1115::Bang::readSample()
{
    Record::Read record ;

    auto q = this->scriptRead(0,&record) ;
    
    RpiExt::Bang scheduler(this->rpi) ;

    scheduler.execute(q.begin(),q.end()) ;

    return record ;
}

Device::Ads1115::Bang::Record::Write Device::Ads1115::Bang::writeConfig(uint16_t word)
{
    Record::Write record ;

    auto q = this->scriptWrite(word,&record) ;
    
    RpiExt::Bang scheduler(this->rpi) ;

    scheduler.execute(q.begin(),q.end()) ;

    return record ;
}

// --------------------------------------------------------------------

uint16_t Device::Ads1115::Bang::fetch(Record::Read const &record) const
{
    uint16_t word = 0 ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    for (size_t i=0 ; i<2 ; ++i)
	for (size_t j=0 ; j<8 ; ++j)
	{
	    word = (uint16_t)(word << 1) ;
	    if (0u != (record.byteA[i][j] & mask))
		word = (uint16_t)(word | 1u) ;
	}
    return word ;
}

Device::Ads1115::Bang::Error Device::Ads1115::Bang::verify(Record::Reset const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    return error ;
}

Device::Ads1115::Bang::Error Device::Ads1115::Bang::verify(Record::Read const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    return error ;
}

Device::Ads1115::Bang::Error Device::Ads1115::Bang::verify(Record::Write const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    if (0 != (record.ackA.at(3) & mask)) error.noAck_3 = 1u ;
    return error ;
}
