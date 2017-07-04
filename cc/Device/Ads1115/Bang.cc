// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"

using Bang = Device::Ads1115::Bang ;

// --------------------------------------------------------------------

void Bang::start(Script *script)
{
    script->sleep(this->timing.buf) ;
    script->low(this->sdaPin) ;
    script->sleep(this->timing.hdsta) ;
    script->low(this->sclPin) ;
}

void Bang::stop(Script *script,Line sda,uint32_t *t0)
{
    script->time(t0) ;
    if (sda != Line::Low)
	script->low(this->sdaPin) ;
    script->sleep(this->timing.sudat) ; 
    script->wait(t0,this->timing.low) ;
    script->off(this->sclPin) ;
    script->sleep(this->timing.susto) ;
    script->off(this->sdaPin) ;
}

// --------------------------------------------------------------------

void Bang::sendBit(Script *script,Line prev,Line next,uint32_t *t0)
{
    script->time(t0) ;
    if (prev != next)
    {
	script->sleep(this->timing.hddat) ; 
	if (next == Line::Low)
	    script->low(this->sdaPin) ;
	else
	    script->off(this->sdaPin) ;
	script->sleep(this->timing.sudat) ; 
    }
    script->wait(t0,this->timing.low) ;
    script->off(this->sclPin) ; 
    script->sleep(this->timing.high) ;
    script->low(this->sclPin) ; 
}

void Bang::recvBit(Script *script,Line sda,uint32_t *t0,uint32_t *levels)
{
    script->time(t0) ;
    if (sda != Line::Off)
	script->off(this->sdaPin) ;
    script->sleep(this->timing.low) ;
    script->off(this->sclPin) ; 
    script->time(t0) ;
    script->wait(t0,this->timing.sudat) ; 
    script->levels(levels) ;
    script->wait(t0,this->timing.high) ; 
    script->low(this->sclPin) ; 
}

// --------------------------------------------------------------------

void Bang::sendByte(
    Script *script,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack)
{
    auto mask = 1u << 7 ;
    do
    {
	auto next = (byte & mask) ? Line::Off : Line::Low ;
	this->sendBit(script,sda,next,t0) ;
	sda = next ;
	mask >>= 1 ;
    }
    while (mask != 0) ;
    this->recvBit(script,sda,t0,ack) ;
}

void Bang::recvByte(
    Script *script,Line sda,uint32_t *t0,Record::Read::Byte *byte)
{
    this->recvBit(script,sda,t0,&byte->at(0)) ;
    for (size_t i=1 ; i<8 ; ++i)
	this->recvBit(script,Line::Off,t0,&byte->at(i)) ;
    this->sendBit(script,Line::Off,Line::Low,t0) ;
}

// --------------------------------------------------------------------

void Bang::reset(Bang::Script *script,Record::Reset *record)
{
    this->start(script) ;
    // (general-address:0x0 + write-bit)
    this->sendByte(script,Line::Low,0x0,&record->t0,&record->ackA.at(0)) ;
    // -> reset-command:0x6
    this->sendByte(script,Line::Off,0x6,&record->t0,&record->ackA.at(1)) ;
    this->stop(script,Line::Off,&record->t0) ;
}

void Bang::read(Bang::Script *script,Record::Read *record,uint8_t reg)
{
    this->start(script) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(script,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register pointer
    this->sendByte(script,Line::Off,reg,&record->t0,&record->ackA.at(1)) ;
    this->stop(script,Line::Off,&record->t0) ;
    
    this->start(script) ;
    // (address + read-bit)
    byte = static_cast<uint8_t>((this->addr.value()<<1) | 1) ;
    this->sendByte(script,Line::Low,byte,&record->t0,&record->ackA.at(2)) ;
    // <- 16-bit register-value
    this->recvByte(script,Line::Off,&record->t0,&record->byteA.at(0)) ;
    this->recvByte(script,Line::Low,&record->t0,&record->byteA.at(1)) ;
    this->stop(script,Line::Low,&record->t0) ;
}

void Bang::write(Bang::Script *script,Record::Write *record,uint16_t word)
{
    this->start(script) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(script,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register-pointer (config-register hard-wired)
    this->sendByte(script,Line::Off,1,&record->t0,&record->ackA.at(1)) ;
    // -> 16-bit register-value
    byte = static_cast<uint8_t>(word>>8) ;
    this->sendByte(script,Line::Off,byte,&record->t0,&record->ackA.at(2)) ;
    byte = static_cast<uint8_t>(word) ;
    this->sendByte(script,Line::Off,byte,&record->t0,&record->ackA.at(3)) ;
    this->stop(script,Line::Off,&record->t0) ;
}

// --------------------------------------------------------------------

Bang::Record::Reset Bang::reset()
{
    Script script ; Record::Reset record ;
    this->reset(&script,&record) ;
    auto vector = script.vector() ;
    RpiExt::Bang(this->rpi).execute(vector.begin(),vector.end()) ;
    return record ;
}

Bang::Record::Read Bang::readConfig()
{
    Script script ; Record::Read record ;
    this->read(&script,&record,/*config-register:*/0x1) ;
    auto vector = script.vector() ;
    RpiExt::Bang(this->rpi).execute(vector.begin(),vector.end()) ;
    return record ;
}

Bang::Record::Read Bang::readSample()
{
    Script script ; Record::Read record ;
    this->read(&script,&record,/*sample-register:*/0x0) ;
    auto vector = script.vector() ;
    RpiExt::Bang(this->rpi).execute(vector.begin(),vector.end()) ;
    return record ;
}

Bang::Record::Write Bang::writeConfig(uint16_t word)
{
    Script script ; Record::Write record ;
    this->write(&script,&record,word) ;
    auto vector = script.vector() ;
    RpiExt::Bang(this->rpi).execute(vector.begin(),vector.end()) ;
    return record ;
}

// --------------------------------------------------------------------

uint16_t Bang::fetch(Record::Read const &record) const
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

Bang::Error Bang::verify(Record::Reset const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    return error ;
}

Bang::Error Bang::verify(Record::Read const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    return error ;
}

Bang::Error Bang::verify(Record::Write const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    if (0 != (record.ackA.at(3) & mask)) error.noAck_3 = 1u ;
    return error ;
}
