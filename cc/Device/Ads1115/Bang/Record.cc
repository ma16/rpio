#include "Record.h"

using namespace Device::Ads1115::Bang::Record ;

Error Reset::verify(Rpi::Pin sda) const
{
    Error error ;
    uint32_t mask = (1u << sda.value()) ;
    if (0 != (this->ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (this->ackA.at(1) & mask)) error.noAck_1 = 1u ;
    return error ;
}

Error Read::verify(Rpi::Pin sda) const
{
    Error error ;
    uint32_t mask = (1u << sda.value()) ;
    if (0 != (this->ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (this->ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (this->ackA.at(2) & mask)) error.noAck_2 = 1u ;
    return error ;
}

uint16_t Read::fetch(Rpi::Pin sda) const
{
    uint16_t word = 0 ;
    uint32_t mask = (1u << sda.value()) ;
    for (size_t i=0 ; i<2 ; ++i)
	for (size_t j=0 ; j<8 ; ++j)
	{
	    word = (uint16_t)(word << 1) ;
	    if (0u != (this->byteA[i][j] & mask))
		word = (uint16_t)(word | 1u) ;
	}
    return word ;
}

Error Write::verify(Rpi::Pin sda) const
{
    Error error ;
    uint32_t mask = (1u << sda.value()) ;
    if (0 != (this->ackA.at(0) & mask)) error.noAck_0 = 1u ;
    if (0 != (this->ackA.at(1) & mask)) error.noAck_1 = 1u ;
    if (0 != (this->ackA.at(2) & mask)) error.noAck_2 = 1u ;
    if (0 != (this->ackA.at(3) & mask)) error.noAck_3 = 1u ;
    return error ;
}
