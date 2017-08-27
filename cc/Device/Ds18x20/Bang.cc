// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;


static uint32_t ticks(double seconds)
{
    return static_cast<uint32_t>(seconds * 250e+6 + .5) ;
}

void Device::Ds18x20::Bang::write(RpiExt::Bang::Enqueue *q,bool bit) const
{
    // [todo] don't use sleep: we need the last absolute point in time
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto low = bit ? this->spec.low1.min : this->spec.low0.min ;
    q->sleep(ticks(low)) ;
    // [todo] verify that max timings aren't exceeded
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto high = this->spec.slot.min + this->spec.rec - low ;
    q->sleep(ticks(high)) ;
    // [todo] verify that max timings aren't exceeded
}

void Device::Ds18x20::Bang::write(RpiExt::Bang::Enqueue *q,uint8_t byte) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	write(q,bit) ;
    }
}

void Device::Ds18x20::Bang::read(RpiExt::Bang::Enqueue *q,uint32_t *levels) const
{
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->sleep(ticks(this->spec.rinit)) ;
    // todo: set LH event register
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    // todo: wait for LH event with 15us timeout
    q->sleep(ticks(this->spec.rrc)) ;
    q->levels(levels) ;
    auto remainder =
	this->spec.slot.min + this->spec.rec -
	this->spec.rinit - this->spec.rrc ;
    q->sleep(ticks(remainder)) ;
}

void Device::Ds18x20::Bang::read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels) const
{
    for (auto i=0u ; i<nwords ; ++i)
    {
	read(q,levels+i) ;
    }
}

void Device::Ds18x20::Bang::init(RpiExt::Bang::Enqueue *q,uint32_t(*t)[4],uint32_t *low,uint32_t *high) const
{
    q->time((*t)+0) ;

    // Reset-Pulse
    
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ; 
    
    q->sleep(ticks(this->spec.rstl)) ;
    
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;

    // Present-Pulse

    q->time((*t)+1) ;

    q->waitLevel(
	(*t)+1,
	ticks(this->spec.pdhigh.max), // 15..60 (wait 60, todo: check 15)
	(*t)+2,
	1u << this->busPin.value(),
	0,
	low) ;

    q->waitLevel(
	(*t)+2,
	ticks(this->spec.pdlow.max), // 60..240 (wait 240, todo: check 60)
	(*t)+3,
	1u << this->busPin.value(),
	1u << this->busPin.value(),
	high) ;

    q->wait((*t)+1,ticks(this->spec.rsth)) ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readRom(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    
    // assumes busPin.mode=Input
    Bang::init(&q,&record->t,&record->low,&record->high) ;
    
    // Read-ROM-Code command
    Bang::write(&q,static_cast<uint8_t>(0x33)) ;
    
    Bang::read(&q,64,record->buffer) ;
    
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readPad(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    
    // assumes busPin.mode=Input
    Bang::init(&q,&record->t,&record->low,&record->high) ;
    
    // Skip-ROM-Code command
    Bang::write(&q,static_cast<uint8_t>(0xcc)) ;

    // Read Sratch-Pad
    Bang::write(&q,static_cast<uint8_t>(0xbe)) ;
    
    Bang::read(&q,128,record->buffer) ;
    
    return q.vector() ;
}

uint8_t Device::Ds18x20::Bang::crc(uint32_t const *buffer,uint32_t mask,size_t nwords)
{
    uint8_t reg = 0 ;
    for (auto i=0u ; i<nwords ; ++i)
    {
	auto lsb = 0 != (reg & 0x01) ;
	reg = static_cast<uint8_t>(reg >> 1) ;
	auto nxt = 0 != (buffer[i] & mask) ;
	if (nxt ^ lsb)
	    reg ^= 0x8c ;
    }
    return reg ;
}
