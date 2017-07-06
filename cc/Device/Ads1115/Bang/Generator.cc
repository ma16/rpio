// BSD 2-Clause License, see github.com/ma16/rpio

#include "Generator.h"

using Generator = Device::Ads1115::Bang::Generator ;

// --------------------------------------------------------------------

void Generator::start(Draft *draft)
{
    draft->sleep(this->config.timing.buf) ;
    draft->low(this->config.sdaPin) ;
    draft->sleep(this->config.timing.hdsta) ;
    draft->low(this->config.sclPin) ;
}

void Generator::stop(Draft *draft,Line sda,uint32_t *t0)
{
    draft->time(t0) ;
    if (sda != Line::Low)
	draft->low(this->config.sdaPin) ;
    draft->sleep(this->config.timing.sudat) ; 
    draft->wait(t0,this->config.timing.low) ;
    draft->off(this->config.sclPin) ;
    draft->sleep(this->config.timing.susto) ;
    draft->off(this->config.sdaPin) ;
}

// --------------------------------------------------------------------

void Generator::sendBit(Draft *draft,Line prev,Line next,uint32_t *t0)
{
    draft->time(t0) ;
    if (prev != next)
    {
	draft->sleep(this->config.timing.hddat) ; 
	if (next == Line::Low)
	    draft->low(this->config.sdaPin) ;
	else
	    draft->off(this->config.sdaPin) ;
	draft->sleep(this->config.timing.sudat) ; 
    }
    draft->wait(t0,this->config.timing.low) ;
    draft->off(this->config.sclPin) ; 
    draft->sleep(this->config.timing.high) ;
    draft->low(this->config.sclPin) ; 
}

void Generator::recvBit(Draft *draft,Line sda,uint32_t *t0,uint32_t *levels)
{
    draft->time(t0) ;
    if (sda != Line::Off)
	draft->off(this->config.sdaPin) ;
    draft->sleep(this->config.timing.low) ;
    draft->off(this->config.sclPin) ; 
    draft->time(t0) ;
    draft->wait(t0,this->config.timing.sudat) ; 
    draft->levels(levels) ;
    draft->wait(t0,this->config.timing.high) ; 
    draft->low(this->config.sclPin) ; 
}

// --------------------------------------------------------------------

void Generator::
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

void Generator::
recvByte(Draft *draft,Line sda,uint32_t *t0,Record::Read::Byte *byte)
{
    this->recvBit(draft,sda,t0,&byte->at(0)) ;
    for (size_t i=1 ; i<8 ; ++i)
	this->recvBit(draft,Line::Off,t0,&byte->at(i)) ;
    this->sendBit(draft,Line::Off,Line::Low,t0) ;
}

// --------------------------------------------------------------------

void Generator::reset(Draft *draft,Record::Reset *record)
{
    this->start(draft) ;
    // (general-address:0x0 + write-bit)
    this->sendByte(draft,Line::Low,0x0,&record->t0,&record->ackA.at(0)) ;
    // -> reset-command:0x6
    this->sendByte(draft,Line::Off,0x6,&record->t0,&record->ackA.at(1)) ;
    this->stop(draft,Line::Off,&record->t0) ;
}

void Generator::read(Draft *draft,Record::Read *record,uint8_t reg)
{
    this->start(draft) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->config.addr.value()<<1) | 0) ;
    this->sendByte(draft,Line::Low,byte,&record->t0,&record->ackA.at(0)) ;
    // -> register pointer
    this->sendByte(draft,Line::Off,reg,&record->t0,&record->ackA.at(1)) ;
    this->stop(draft,Line::Off,&record->t0) ;
    
    this->start(draft) ;
    // (address + read-bit)
    byte = static_cast<uint8_t>((this->config.addr.value()<<1) | 1) ;
    this->sendByte(draft,Line::Low,byte,&record->t0,&record->ackA.at(2)) ;
    // <- 16-bit register-value
    this->recvByte(draft,Line::Off,&record->t0,&record->byteA.at(0)) ;
    this->recvByte(draft,Line::Low,&record->t0,&record->byteA.at(1)) ;
    this->stop(draft,Line::Low,&record->t0) ;
}

void Generator::readConfig(Draft *draft,Record::Read *record)
{
    this->read(draft,record,/*config-register:*/0x1) ;
}

void Generator::readSample(Draft *draft,Record::Read *record)
{
    this->read(draft,record,/*sample-register:*/0x0) ;
}

void Generator::writeConfig(Draft *draft,Record::Write *record,uint16_t word)
{
    this->start(draft) ;
    // (address + write-bit)
    auto byte = static_cast<uint8_t>((this->config.addr.value()<<1) | 0) ;
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

Generator::Script Generator::reset(Record::Reset *record)
{
    Draft draft ;
    this->reset(&draft,record) ;
    return draft.vector() ;
}

Generator::Script Generator::readConfig(Record::Read *record)
{
    Draft draft ;
    this->readConfig(&draft,record) ;
    return draft.vector() ;
}

Generator::Script Generator::readSample(Record::Read *record)
{
    Draft draft ;
    this->readSample(&draft,record) ;
    return draft.vector() ;
}

Generator::Script Generator::writeConfig(Record::Write *record,uint16_t word)
{
    Draft draft ;
    this->writeConfig(&draft,record,word) ;
    return draft.vector() ;
}
