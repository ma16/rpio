// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <Protocol/OneWire/Bang/Error.h>

using Bang = Device::Ds18b20::Bang ;

using Error = Protocol::OneWire::Bang::Error ;

void Bang::convert(boost::optional<Address> const &address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Convert-T
    this->signaling.write(std::bitset<8>(0x44)) ;
    // ...this may take a while; client can issue Read Time-Slot
}

bool Bang::isBusy()
{
    return this->signaling.isBusy() ;
}

bool Bang::isPowered(boost::optional<Address> const& address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Read-Power-Supply
    this->signaling.write(std::bitset<8>(0xb4)) ;
    // Read-Time-Slot returns power mode
    return this->signaling.read() ;
}

Bang::Pad Bang::readPad(boost::optional<Address> const& address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Read-Sratch-Pad
    this->signaling.write(std::bitset<8>(0xbe)) ;
    return this->signaling.read<72>() ;
}

void Bang::restoreThresholds(boost::optional<Address> const &address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Recall E2
    this->signaling.write(std::bitset<8>(0xb8)) ;
}

void Bang::saveThresholds(boost::optional<Address> const &address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Copy Sratch-Pad
    this->signaling.write(std::bitset<8>(0x48)) ;
}

void Bang::writeThresholds(uint16_t thr,boost::optional<Address> const &address)
{
    auto present = this->signaling.init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    if (address)
    {
	this->signaling.write(Master::MatchRom) ;
	this->signaling.write(*address) ;
    }	
    else this->signaling.write(Master::SkipRom) ;
    // Function-command: Write-Sratch-Pad
    this->signaling.write(std::bitset<8>(0x4e)) ;
    this->signaling.write(std::bitset<16>(thr)) ;
}
