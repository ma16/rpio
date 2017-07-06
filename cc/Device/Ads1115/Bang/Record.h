// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ads1115_Bang_Record_h
#define INCLUDE_Device_Ads1115_Bang_Record_h

#include <array>
#include <Rpi/Pin.h>

namespace Device { namespace Ads1115 { namespace Bang {

namespace Record
{
    struct Error
    {
	uint8_t noAck_0 : 1 ; 
	uint8_t noAck_1 : 1 ;
	uint8_t noAck_2 : 1 ;
	uint8_t noAck_3 : 1 ;
	
	Error() : noAck_0(0),noAck_1(0),noAck_2(0),noAck_3(0) {}
	
	bool success() const
	{
	    return 0 == (noAck_0 | noAck_1 | noAck_2 | noAck_3) ;
	}
    } ;

    struct Reset
    {
	uint32_t t0 ;
	std::array<uint32_t,2> ackA ;
	Error verify(Rpi::Pin sda) const ;
    } ;
    
    struct Read
    {
	uint32_t t0 ;
	
	std::array<uint32_t,3> ackA ;
	
	using Byte = std::array<uint32_t,8> ;
	
	std::array<Byte,2> byteA ;
	
	uint16_t fetch(Rpi::Pin sda) const ;
	
	Error verify(Rpi::Pin sda) const ;
    } ;
    
    struct Write
    {
	uint32_t t0 ;
	std::array<uint32_t,4> ackA ;
	Error verify(Rpi::Pin sda) const ;
    } ;
} 

} } }

#endif // INCLUDE_Device_Ads1115_Bang_Record_h
