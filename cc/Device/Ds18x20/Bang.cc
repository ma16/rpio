// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;

Device::Ds18x20::Bang::Timing<uint32_t>
Device::Ds18x20::Bang::ticks(Timing<double> const &seconds,double tps)
{
    Device::Ds18x20::Bang::Timing<uint32_t> ticks ;
    auto f = [tps](double s) { return static_cast<uint32_t>(s * tps + 0.5) ; } ;
    
    ticks.slot.min = f(seconds.slot.min) ;
    ticks.slot.max = f(seconds.slot.max) ;

    ticks.rec = f(seconds.rec) ;
    
    ticks.low0.min = f(seconds.low0.min) ;
    ticks.low0.max = f(seconds.low0.max) ;

    ticks.low1.min = f(seconds.low1.min) ;
    ticks.low1.max = f(seconds.low1.max) ;

    ticks.rinit = f(seconds.rinit) ;
    ticks.rrc = f(seconds.rrc) ;
    ticks.rdv = f(seconds.rdv) ;
    ticks.rsth = f(seconds.rsth) ;
    ticks.rstl = f(seconds.rstl) ;
    
    ticks.pdhigh.min = f(seconds.pdhigh.min) ;
    ticks.pdhigh.max = f(seconds.pdhigh.max) ;

    ticks.pdlow.min = f(seconds.pdlow.min) ;
    ticks.pdlow.max = f(seconds.pdlow.max) ;

    return ticks ;
}

void Device::Ds18x20::Bang::write(RpiExt::Bang::Enqueue *q,bool bit) const
{
    // [todo] don't use sleep: we need the last absolute point in time
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto low = bit ? this->timing.low1.min : this->timing.low0.min ;
    q->sleep(low) ;
    // [todo] verify that max timings aren't exceeded
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto high = this->timing.slot.min + this->timing.rec - low ;
    q->sleep(high) ;
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
    q->sleep(this->timing.rinit) ;
    // todo: set LH event register
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    // todo: wait for LH event with 15us timeout
    q->sleep(this->timing.rrc) ;
    q->levels(levels) ;
    auto remainder =
	this->timing.slot.min + this->timing.rec -
	this->timing.rinit - this->timing.rrc ;
    q->sleep(remainder) ;
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
    
    q->sleep(this->timing.rstl) ;
    
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;

    // Present-Pulse

    q->time((*t)+1) ;

    q->waitLevel(
	(*t)+1,
	this->timing.pdhigh.max, // 15..60 (wait 60, todo: check 15)
	(*t)+2,
	1u << this->busPin.value(),
	0,
	low) ;

    q->waitLevel(
	(*t)+2,
	this->timing.pdlow.max, // 60..240 (wait 240, todo: check 60)
	(*t)+3,
	1u << this->busPin.value(),
	1u << this->busPin.value(),
	high) ;

    q->wait((*t)+1,this->timing.rsth) ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readRom(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    this->init(&q,&record->t,&record->low,&record->high) ;
    // ROM-command: Read-ROM-Code
    this->write(&q,static_cast<uint8_t>(0x33)) ;
    this->read(&q,64,record->buffer) ;
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readPad(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    this->init(&q,&record->t,&record->low,&record->high) ;
    // ROM-command: Skip-ROM-Code
    this->write(&q,static_cast<uint8_t>(0xcc)) ;
    // Function-command: Read-Sratch-Pad
    this->write(&q,static_cast<uint8_t>(0xbe)) ;
    Bang::read(&q,72,record->buffer) ;
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
