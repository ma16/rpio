// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;


std::vector<bool> Device::Ds18x20::Bang::
assemble(uint32_t const *buffer,size_t nwords,uint32_t mask)
{
    std::vector<bool> v(nwords) ;
    for (decltype(nwords) i=0 ; i<nwords ; ++i)
    {
	v[i] = 0 != (buffer[i] & mask) ;
    }
    return v ;
}

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

void Device::Ds18x20::Bang::
write(RpiExt::Bang::Enqueue *q,bool bit,uint32_t (*t)[2]) const
{
    q->time((*t)+0) ; // auto t0 = q->timeAlloc() ;
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->time((*t)+1) ; // auto t1 = q->timeAlloc() ;
    auto range = bit ? this->timing.low1 : this->timing.low0 ;
    q->wait((*t)+1,range.min) ; // q->wait(t1,range.min) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->assume((*t)+0,range.max) ; // q->assume(t0,range.max) ; 
    q->wait((*t)+1,this->timing.slot.min + this->timing.rec) ;
    //q->wait(t1,this->timing.slot.min + this->timing.rec) ;
    // ...don't care if we exceed slot.max since the bus is idle anyway
    //q->release(t1) ;
    //q->release(t0) ;
}

void Device::Ds18x20::Bang::
write(RpiExt::Bang::Enqueue *q,uint8_t byte,uint32_t (*t)[2]) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	write(q,bit,t) ;
    }
}

void Device::Ds18x20::Bang::
read(RpiExt::Bang::Enqueue *q,uint32_t *levels,uint32_t (*t)[2]) const
{
    q->time((*t)+0) ; // auto t0 = q->timeAlloc() ;
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->time((*t)+1) ; // auto t1 = q->timeAlloc() ;
    q->wait((*t)+1,this->timing.rinit) ; // q->wait(t1,this->timing.rinit) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(this->timing.rrc) ;
    q->levels(levels) ; // [todo] ALLOC and RETURN location
    // ...there might be better ways to record the signal, especially
    // since we want to record it as late as possible (but before RDV
    // expires!).
    q->assume((*t)+0,this->timing.rdv) ; // q->assume(t0,this->timing.rdv) ;
    q->wait((*t)+1,this->timing.slot.min + this->timing.rec) ;
    //q->wait(t1,this->timing.slot.min + this->timing.rec) ;
    // ...don't care if we exceed slot.max since the bus is idle anyway
    //q->release(t1) ;
    //q->release(t0) ;
}

void Device::Ds18x20::Bang::
read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels,uint32_t (*t)[2]) const
{
    for (auto i=0u ; i<nwords ; ++i)
    {
	read(q,levels+i,t) ;
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

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::convert(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    this->init(&q,&record->t,&record->low,&record->high) ;
    // ROM-command: Skip-ROM-Code
    this->write(&q,static_cast<uint8_t>(0xcc),&record->temp) ;
    // Function-command: Convert-T
    this->write(&q,static_cast<uint8_t>(0x44),&record->temp) ;
    // [todo]
    //   here we can issue Read-Time-Slot until 1;
    //   we would need our script to loop
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readRom(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    this->init(&q,&record->t,&record->low,&record->high) ;
    // ROM-command: Read-ROM-Code
    this->write(&q,static_cast<uint8_t>(0x33),&record->temp) ;
    this->read(&q,64,record->buffer,&record->temp) ;
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::readPad(Record *record) const
{
    RpiExt::Bang::Enqueue q ;
    this->init(&q,&record->t,&record->low,&record->high) ;
    // ROM-command: Skip-ROM-Code
    this->write(&q,static_cast<uint8_t>(0xcc),&record->temp) ;
    // Function-command: Read-Sratch-Pad
    this->write(&q,static_cast<uint8_t>(0xbe),&record->temp) ;
    Bang::read(&q,72,record->buffer,&record->temp) ;
    return q.vector() ;
}
