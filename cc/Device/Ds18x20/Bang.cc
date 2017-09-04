// BSD 2-Clause License, see github.com/ma16/rpio

#include <Device/Ds18x20/Bang.h>

constexpr Device::Ds18x20::Bang::Timing<double> Device::Ds18x20::Bang::spec ;

void Device::Ds18x20::Bang::init(RpiExt::BangIo *io) const
{
    //  t0 t1 t2 t3 t4 t5 t6 t7
    // ---+     +-----+     +----...
    //    |     |     |     |
    //    +-----+     +-----+
    
    // tx: Reset-Pulse
    io->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    io->sleep(this->timing.rstl) ;
    auto t2 = io->time() ; 
    io->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    
    // rx: wait for HL-edge (start of Presence-Pulse)
    auto t3 = io->time() ;
    auto t4 = io->waitFor(t2,this->timing.pdhigh.max,this->pinMask,/*Low*/0) ;
    auto t5 = io->recent() ;
    if (t5 - t2 > this->timing.pdhigh.max)
	throw Error(std::to_string(__LINE__)) ;
    if (t4 - t3 < this->timing.pdhigh.min)
	throw Error(std::to_string(__LINE__)) ;
    
    // rx: wait for LH-edge (end of Presence-Pulse)
    auto t6 = io->waitFor(t4,
			  this->timing.pdlow.max,
			  this->pinMask,
			  /*High*/this->pinMask) ;
    auto t7 = io->recent() ;
    if (t7 - t4 > this->timing.pdlow.max)
	throw Error(std::to_string(__LINE__)) ;
    if (t6 - t5 < this->timing.pdlow.min)
	throw Error(std::to_string(__LINE__)) ;

    // rx: wait for end of Presence-Pulse cycle
    io->wait(t3,this->timing.rsth) ;
}

void Device::Ds18x20::Bang::write(RpiExt::BangIo *io,bool bit) const
{
    //  t0 t1 t2 t3
    // ---+     +---
    //    |     |
    //    +-----+

    // tx: Bit Pulse
    auto t0 = io->time() ;
    io->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = io->time() ;
    auto range = bit ? this->timing.low1 : this->timing.low0 ;
    io->wait(t1,range.min) ; 
    io->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = io->time() ; 
    if (t3 - t0 > range.max) 
	throw Error(std::to_string(__LINE__)) ;

    // minimum recovery time (after LH edge)
    io->wait(t3,this->timing.rec) ;
	
    // rx: wait for end of cycle
    io->wait(t1,this->timing.slot.min) ;
}

void Device::Ds18x20::Bang::write(RpiExt::BangIo *io,uint8_t byte) const
{
    for (auto i=0u ; i<8u ; ++i)
    {
	auto bit = 0 != (byte & (1u<<i)) ;
	this->write(io,bit) ;
    }
}

bool Device::Ds18x20::Bang::read(RpiExt::BangIo *io) const
{
    //  t0 t1 t2 t3 t4 t5
    // ---+     +-----+---
    //    |     |     |   
    //    +-----+-----+

    // tx: initiate Read-Time-Slot
    auto t0 = io->time() ; 
    io->mode(this->busPin,Rpi::Gpio::Mode::Out) ;
    auto t1 = io->time() ; 
    io->wait(t1,this->timing.rinit.min) ;
    io->mode(this->busPin,Rpi::Gpio::Mode::In) ;
    auto t3 = io->time() ; 

    // rx: wait for LH edge 
    io->waitFor(t1,this->timing.slot.max,this->pinMask,/*High*/this->pinMask) ;
    auto t5 = io->recent() ; 

    // make sure we got not interrupted while HL edge
    if (t1 - t0 > this->timing.rinitgap)
	throw Error(std::to_string(__LINE__)) ;

    // make sure we got not interrupted when releasing the bus
    if (t3 - t0 > this->timing.rinit.max) 
	throw Error(std::to_string(__LINE__)) ;
    // make sure we didn't reset the bus
    if (t3 - t0 > this->timing.rstl/2) 
	throw Error("reset") ;

    // what did we receive? A one- or a zero-bit?
    auto bit = (t5 - t0) <= this->timing.rdv ; 
    
    // minimum recovery time (after LH edge)
    io->wait(t5,this->timing.rec) ;
	
    // rx: wait for end of cycle
    io->wait(t1,this->timing.slot.min) ;

    return bit ;
}

void Device::Ds18x20::Bang::
read(RpiExt::BangIo *io,size_t nbits,bool *bitA) const
{
    for (auto i=0u ; i<nbits ; ++i)
    {
	bitA[i] = this->read(io) ;
    }
}

void Device::Ds18x20::Bang::convert(RpiExt::BangIo *io) const
{
    this->init(io) ;
    // ROM-command: Skip-ROM-Code
    this->write(io,static_cast<uint8_t>(0xcc)) ;
    // Function-command: Convert-T
    this->write(io,static_cast<uint8_t>(0x44)) ;
    // [todo]
    //   here we can issue Read-Time-Slot until 1;
    //   we would need our script to loop
}

void Device::Ds18x20::Bang::readPad(RpiExt::BangIo *io,bool(*rx)[72]) const
{
    this->init(io) ;
    // ROM-command: Skip-ROM-Code
    this->write(io,static_cast<uint8_t>(0xcc)) ;
    // Function-command: Read-Sratch-Pad
    this->write(io,static_cast<uint8_t>(0xbe)) ;
    Bang::read(io,72,*rx) ;
}

void Device::Ds18x20::Bang::readRom(RpiExt::BangIo *io,bool(*rx)[64]) const
{
    this->init(io) ;
    // ROM-command: Read-ROM-Code
    this->write(io,static_cast<uint8_t>(0x33)) ;
    this->read(io,64,*rx) ;
}

bool Device::Ds18x20::Bang::isBusy(RpiExt::BangIo *io) const
{
    return !this->read(io) ;
}

#if 0

// this doesn't work with script generator; at least not without
// heavy modifications to the script generator

? searchRom(Stack *stack,bool(*rx)[64]) const
{
    this->init(&q,stack) ;
    // ROM-command: Search Read-ROM-Code
    this->write(&q,stack,static_cast<uint8_t>(0xf0)) ;
    for (auto i=0u ; i<64 ; ++i)
    {
	auto sp = stack->save() ;
	//auto bit = reinterpret_cast<bool*>(stack.push()) ;
	//this->read(&q,stack,bit) ;
	this->read(&q,stack,(*rx)+i) ;
	auto inv = reinterpret_cast<bool*>(stack->push()) ;
	this->read(&q,stack,inv) ;

	// if 0-1: branch into 0
	// if 1-0: branch into 1
	q->call((*rx)+i,write1,write0) ;

	// if 1-1: end of list ; no device (bit0) or device removed
	
	//this->write(&q,stack,(*rx)+i) ;
	//this->write(&q,stack,inv) ;
	
	// if 0-0:
	// ...
	stack->recover(sp) ;
    }
    return q.vector() ;
}
#endif

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

    ticks.rinit.min = f(seconds.rinit.min) ;
    ticks.rinit.max = f(seconds.rinit.max) ;
    ticks.rinitgap = f(seconds.rinitgap) ;
    ticks.rdv = f(seconds.rdv) ;
    ticks.rsth = f(seconds.rsth) ;
    ticks.rstl = f(seconds.rstl) ;
    
    ticks.pdhigh.min = f(seconds.pdhigh.min) ;
    ticks.pdhigh.max = f(seconds.pdhigh.max) ;

    ticks.pdlow.min = f(seconds.pdlow.min) ;
    ticks.pdlow.max = f(seconds.pdlow.max) ;

    return ticks ;
}
