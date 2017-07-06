// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"

using Bang = Device::Ads1115::Bang ;

// --------------------------------------------------------------------

void Bang::Generator::start(Draft *draft)
{
    draft->sleep(this->timing.buf) ;
    draft->low(this->sdaPin) ;
    draft->sleep(this->timing.hdsta) ;
    draft->low(this->sclPin) ;
}

void Bang::Generator::stop(Draft *draft,Line sda,uint32_t *t0)
{
    draft->time(t0) ;
    if (sda != Line::Low)
	draft->low(this->sdaPin) ;
    draft->sleep(this->timing.sudat) ; 
    draft->wait(t0,this->timing.low) ;
    draft->off(this->sclPin) ;
    draft->sleep(this->timing.susto) ;
    draft->off(this->sdaPin) ;
}

// --------------------------------------------------------------------

void Bang::Generator::sendBit(Draft *draft,Line prev,Line next,uint32_t *t0)
{
    draft->time(t0) ;
    if (prev != next)
    {
	draft->sleep(this->timing.hddat) ; 
	if (next == Line::Low)
	    draft->low(this->sdaPin) ;
	else
	    draft->off(this->sdaPin) ;
	draft->sleep(this->timing.sudat) ; 
    }
    draft->wait(t0,this->timing.low) ;
    draft->off(this->sclPin) ; 
    draft->sleep(this->timing.high) ;
    draft->low(this->sclPin) ; 
}

void Bang::Generator::
recvBit(Draft *draft,Line sda,uint32_t *t0,uint32_t *levels)
{
    draft->time(t0) ;
    if (sda != Line::Off)
	draft->off(this->sdaPin) ;
    draft->sleep(this->timing.low) ;
    draft->off(this->sclPin) ; 
    draft->time(t0) ;
    draft->wait(t0,this->timing.sudat) ; 
    draft->levels(levels) ;
    draft->wait(t0,this->timing.high) ; 
    draft->low(this->sclPin) ; 
}

// --------------------------------------------------------------------

void Bang::Generator::
sendByte(Draft *draft,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack)
{
    auto mask = 1u << 7 ;
    do
    {
	auto next = (byte & mask) ? Line::Off : Line::Low ;
	this->sendBit(draft,sda,next,t0) ;
	sda = next ;
	mask >>= 1 ;
    }
    while (mask != 0) ;
    this->recvBit(draft,sda,t0,ack) ;
}

void Bang::Generator::
recvByte(Draft *draft,Line sda,uint32_t *t0,Record::Read::Byte *byte)
{
    this->recvBit(draft,sda,t0,&byte->at(0)) ;
    for (size_t i=1 ; i<8 ; ++i)
	this->recvBit(draft,Line::Off,t0,&byte->at(i)) ;
    this->sendBit(draft,Line::Off,Line::Low,t0) ;
}

// --------------------------------------------------------------------

void Bang::Generator::reset(Draft *draft,Record::Reset *record)
{
    this->start(draft) ;
    // (general-address:0x0 + write-bit)
    this->sendByte(draft,Line::Low,0x0,&record->t0,&record->ackA.at(0)) ;
    // -> reset-command:0x6
    this->sendByte(draft,Line::Off,0x6,&record->t0,&record->ackA.at(1)) ;
    this->stop(draft,Line::Off,&record->t0) ;
}

void Bang::Generator::read(Draft *draft,Record::Read *record,uint8_t reg)
{
    this->start(draft) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(draft,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register pointer
    this->sendByte(draft,Line::Off,reg,&record->t0,&record->ackA.at(1)) ;
    this->stop(draft,Line::Off,&record->t0) ;
    
    this->start(draft) ;
    // (address + read-bit)
    byte = static_cast<uint8_t>((this->addr.value()<<1) | 1) ;
    this->sendByte(draft,Line::Low,byte,&record->t0,&record->ackA.at(2)) ;
    // <- 16-bit register-value
    this->recvByte(draft,Line::Off,&record->t0,&record->byteA.at(0)) ;
    this->recvByte(draft,Line::Low,&record->t0,&record->byteA.at(1)) ;
    this->stop(draft,Line::Low,&record->t0) ;
}

void Bang::Generator::readConfig(Draft *draft,Record::Read *record)
{
    this->read(draft,record,/*config-register:*/0x1) ;
}

void Bang::Generator::readSample(Draft *draft,Record::Read *record)
{
    this->read(draft,record,/*sample-register:*/0x0) ;
}

void Bang::Generator::writeConfig(Draft *draft,Record::Write *record,uint16_t word)
{
    this->start(draft) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->addr.value()<<1) | 0) ;
    this->sendByte(draft,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register-pointer (config-register hard-wired)
    this->sendByte(draft,Line::Off,1,&record->t0,&record->ackA.at(1)) ;
    // -> 16-bit register-value
    byte = static_cast<uint8_t>(word>>8) ;
    this->sendByte(draft,Line::Off,byte,&record->t0,&record->ackA.at(2)) ;
    byte = static_cast<uint8_t>(word) ;
    this->sendByte(draft,Line::Off,byte,&record->t0,&record->ackA.at(3)) ;
    this->stop(draft,Line::Off,&record->t0) ;
}

// --------------------------------------------------------------------

Bang::Generator::Script Bang::Generator::reset(Record::Reset *record)
{
    Draft draft ;
    this->reset(&draft,record) ;
    return draft.vector() ;
}

Bang::Generator::Script Bang::Generator::readConfig(Record::Read *record)
{
    Draft draft ;
    this->readConfig(&draft,record) ;
    return draft.vector() ;
}

Bang::Generator::Script Bang::Generator::readSample(Record::Read *record)
{
    Draft draft ;
    this->readSample(&draft,record) ;
    return draft.vector() ;
}

Bang::Generator::Script
Bang::Generator::writeConfig(Record::Write *record,uint16_t word)
{
    Draft draft ;
    this->writeConfig(&draft,record,word) ;
    return draft.vector() ;
}

// --------------------------------------------------------------------

Bang::Record::Reset Bang::reset()
{
    Record::Reset record ;
    auto script = this->gen.reset(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Read Bang::readConfig()
{
    Record::Read record ;
    auto script = this->gen.readConfig(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Read Bang::readSample()
{
    Record::Read record ;
    auto script = this->gen.readSample(&record) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

Bang::Record::Write Bang::writeConfig(uint16_t word)
{
    Record::Write record ;
    auto script = this->gen.writeConfig(&record,word) ;
    RpiExt::Bang(this->rpi).execute(script) ;
    return record ;
}

// --------------------------------------------------------------------

uint16_t Bang::fetch(Record::Read const &record) const
{
    uint16_t word = 0 ;
    uint32_t mask = (1u << this->gen.sdaPin.value()) ;
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
    uint32_t mask = (1u << this->gen.sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    return error ;
}

Bang::Error Bang::verify(Record::Read const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->gen.sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    return error ;
}

Bang::Error Bang::verify(Record::Write const &record) const 
{
    Error error ;
    uint32_t mask = (1u << this->gen.sdaPin.value()) ;
    if (0 != (record.ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (record.ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (record.ackA.at(2) & mask)) error.noAck_2 = 1u ;
    if (0 != (record.ackA.at(3) & mask)) error.noAck_3 = 1u ;
    return error ;
}
