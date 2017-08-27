// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef INCLUDE_Device_Ds18x20_Bang_h
#define INCLUDE_Device_Ds18x20_Bang_h

#include <RpiExt/Bang.h>

namespace Device { namespace Ds18x20 {

struct Bang
{
    Bang(
	Rpi::Peripheral *rpi,
	Rpi::Pin      busPin)
	
	: rpi(rpi),busPin(busPin) {}

    using Script = std::vector<RpiExt::Bang::Command> ;

    struct Record
    {
	uint32_t t0 ;
	uint32_t t1 ;
	uint32_t t2 ;
	uint32_t t3 ;

	uint32_t low ;
	uint32_t high ;

	uint32_t buffer[0x1000] ;
    } ;
    
    Script makeScript(Record *record) const ;
    Script makeScript2(Record *record) const ;
    
    static uint8_t crc(uint32_t const *buffer,uint32_t mask,size_t nwords) ;
private:
  
    void read(RpiExt::Bang::Enqueue *q,uint32_t *levels) const ;
    void read(RpiExt::Bang::Enqueue *q,size_t nwords,uint32_t *levels) const ;
    
    void write(RpiExt::Bang::Enqueue *q,bool bit) const ;
    void write(RpiExt::Bang::Enqueue *q,uint8_t byte) const ;

    Rpi::Peripheral *rpi ;

    Rpi::Pin busPin ;

} ; } } 

#endif // INCLUDE_Device_Ds18x20_Bang_h
 
