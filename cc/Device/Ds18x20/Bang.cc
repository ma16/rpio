// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>
#include <cstring> // memset

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;

void Device::Ds18x20::Bang::init(Enqueue *q,Stack *stack) const
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    auto sp = stack->save() ;
    using Op = RpiExt::Bang::Command::Assume::Op ;
    
    // tx: Reset-Pulse
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ; 
    q->sleep(this->timing.rstl) ;
    auto t2 = stack->push() ; q->time(t2) ; 
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    
    // rx: wait for HL-edge (start of Presence-Pulse)
    auto t3 = stack->push() ; q->time(t3) ;
    auto t4 = stack->push() ; 
    q->waitFor(t2,this->timing.pdhigh.max,this->busPin,/*Low*/0,t4) ;
    auto t5 = stack->push() ; q->recent(t5) ;
    auto duration = stack->push() ;
    q->duration(t2,t5,duration) ;
    q->assume(duration,Op::Le,this->timing.pdhigh.max,45) ;
    q->duration(t3,t4,duration) ;
    q->assume(duration,Op::Ge,this->timing.pdhigh.min,46) ;
    
    // rx: wait for LH-edge (end of Presence-Pulse)
    auto t6 = stack->push() ; 
    q->waitFor(t4,this->timing.pdlow.max,this->busPin,/*High*/1,t6) ;
    auto t7 = stack->push() ; q->recent(t7) ;
    q->duration(t4,t7,duration) ;
    q->assume(duration,Op::Le,this->timing.pdlow.max,47) ;
    q->duration(t5,t6,duration) ;
    q->assume(duration,Op::Ge,this->timing.pdlow.min,48) ;

    // rx: wait for end of Present-Pulse cycle
    q->wait(t3,this->timing.rsth) ;
    
    stack->recover(sp) ;
}

void Device::Ds18x20::Bang::
write(Enqueue *q,Stack *stack,bool bit) const
{
    //  t0 t1    t2
    // ---+     +-----
    //    |     |
    //    +-----+

    auto t0 = stack->push() ; q->time(t0) ;
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = stack->push() ; q->time(t1) ; 
    auto range = bit ? this->timing.low1 : this->timing.low0 ;
    q->wait(t1,range.min) ; 
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;

    // make sure the low time doesn't exceed max limit
    //auto t2 = stack->push() ; q->time(t1) ; 
    //q->assume(t0,range.max,3) ; 

    // make sure the slot has a minimum length
    q->wait(t1,this->timing.slot.min + this->timing.rec) ;
    // ...don't care if we exceed slot.max since the bus is idle anyway
    stack->pop(2) ;
}

void Device::Ds18x20::Bang::
write(Enqueue *q,Stack *stack,uint8_t byte) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	write(q,stack,bit) ;
    }
}

void Device::Ds18x20::Bang::
read(Enqueue *q,Stack *stack,uint32_t *levels) const
{
    auto t0 = stack->push() ;
    auto t1 = stack->push() ;
    q->time(t0) ; 
    q->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    q->time(t1) ; 
    q->wait(t1,this->timing.rinit) ; 
    q->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    q->sleep(this->timing.rrc) ;
    q->levels(levels) ; // [todo] ALLOC and RETURN location
    // ...there might be better ways to record the signal, especially
    // since we want to record it as late as possible (but before RDV
    // expires!).
    //q->assume(t0,this->timing.rdv,4) ; 
    q->wait(t1,this->timing.slot.min + this->timing.rec) ;
    // ...don't care if we exceed slot.max since the bus is idle anyway
    stack->pop(2) ;
}

void Device::Ds18x20::Bang::
read(Enqueue *q,Stack *stack,size_t nwords,uint32_t *levels) const
{
    for (auto i=0u ; i<nwords ; ++i)
    {
	read(q,stack,levels+i) ;
    }
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::
convert(Stack *stack) const
{
    Enqueue q ;
    this->init(&q,stack) ;
    // ROM-command: Skip-ROM-Code
    this->write(&q,stack,static_cast<uint8_t>(0xcc)) ;
    // Function-command: Convert-T
    this->write(&q,stack,static_cast<uint8_t>(0x44)) ;
    // [todo]
    //   here we can issue Read-Time-Slot until 1;
    //   we would need our script to loop
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::
readPad(Stack *stack,uint32_t(*rx)[72]) const
{
    Enqueue q ;
    this->init(&q,stack) ;
    // ROM-command: Skip-ROM-Code
    this->write(&q,stack,static_cast<uint8_t>(0xcc)) ;
    // Function-command: Read-Sratch-Pad
    this->write(&q,stack,static_cast<uint8_t>(0xbe)) ;
    Bang::read(&q,stack,72,*rx) ;
    return q.vector() ;
}

Device::Ds18x20::Bang::Script Device::Ds18x20::Bang::
readRom(Stack *stack,uint32_t(*rx)[64]) const
{
    Enqueue q ;
    this->init(&q,stack) ;
    // ROM-command: Read-ROM-Code
    this->write(&q,stack,static_cast<uint8_t>(0x33)) ;
    this->read(&q,stack,64,*rx) ;
    return q.vector() ;
}

void Device::Ds18x20::Bang::
pack(uint32_t const from[],size_t nwords,uint32_t mask,char to[])
{
    memset(to,0x0,(nwords+7)/8) ;
    for (decltype(nwords) i=0 ; i<nwords ; ++i)
    {
	auto bit = 0 != (from[i] & mask) ;
	if (bit)
	    to[i/8] |= static_cast<char>(1 << (i % 8)) ;
    }
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
