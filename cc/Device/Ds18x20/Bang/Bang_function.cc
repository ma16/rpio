// BSD 2-Clause License, see github.com/ma16/rpio

#include "Bang.h"
#include <Protocol/OneWire/Bang/Error.h>

using Bang = Device::Ds18x20::Bang::Bang ;

using Error = Protocol::OneWire::Bang::Error ;

void Bang::convert()
{
    auto present = this->master->init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    this->master->write(Master::SkipRom) ;
    // Function-command: Convert-T
    this->master->write(std::bitset<8>(0x44)) ;
    // [todo]
    //   here we can issue Read-Time-Slot until 1;
    //   we would need our script to loop
}

Bang::Pad Bang::readPad()
{
    auto present = this->master->init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    this->master->write(Master::SkipRom) ;
    // Function-command: Read-Sratch-Pad
    this->master->write(std::bitset<8>(0xbe)) ;
    return this->master->read<72>() ;
}

bool Bang::isBusy()
{
    return !this->master->read() ;
}

bool Bang::isPowered()
{
    auto present = this->master->init() ;
    if (!present)
	throw Error(Error::Type::NotPresent,__LINE__) ;
    this->master->write(Master::SkipRom) ;
    // Function-command: Read-Power-Supply
    this->master->write(std::bitset<8>(0xb4)) ;
    // Read-Time-Slot returns power mode
    return this->master->read() ;
}
