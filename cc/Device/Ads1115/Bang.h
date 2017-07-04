// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// The ADS1115 is an I2C driven 15-bit (+sign) ADC with 2x2 or 4x1
// multiplexed channels and a maximum sampling rate of somewhat below
// 1k/s.
//
// This implementation does only support the "single-shot" mode with
// an up to 400khz bus clock (bit-banged).
//
// This implementation does not support:
// --continuous mode
// --comparator configuration
// --high speed I2C bus clock
//
// Please refer to the ADS1115 datasheet for more details.
// --------------------------------------------------------------------

#ifndef INCLUDE_Device_Ads1115_Host_h
#define INCLUDE_Device_Ads1115_Host_h

#include <Neat/Enum.h>
#include <RpiExt/Bang.h>

namespace Device { namespace Ads1115 {

struct Bang
{
    using Source = Neat::Enum<unsigned,0x7> ;
    //     |  Pos |  Neg
    // ----+------+------------
    // 000 | IN-0 | IN-1 (default)
    // 001 | IN-0 | IN-3
    // 010 | IN-1 | IN-3
    // 011 | IN-2 | IN-3
    // 100 | IN-0 |  GND
    // 101 | IN-1 |  GND
    // 110 | IN-2 |  GND
    // 111 | IN-3 |  GND

    using Sample = int16_t ;

    using Addr = Neat::Enum<unsigned,0x7f> ;

    template<typename T> struct Timing 
    {
	T buf   ; // (min) bus free time between START and STOP condition
	T hdsta ; // (min) hold time after repeated START condition.
	T susto ; // (min) stop condition setup time
	T sudat ; // (min) data setup time
	T hddat ; // (min) data hold time
	T low   ; // (min) SCL clock low period
	T high  ; // (min) SCL clock high period

	/* there are also maximum fall and raise time constraints for 
	   the SCL edges (300ns) which we could verify, however, not 
	   at this moment. */
	
	Timing(T buf,T hdsta,T susto,T sudat,T hddat,T low,T high) :
	    buf    (buf), hdsta(hdsta), susto(susto), sudat(sudat),
	    hddat(hddat), low    (low), high  (high) {}
    } ;

    static Timing<float> const& fast_timing()
    {
	static Timing<float> t =
	{ 6e-7f,6e-7f,6e-7f,1e-7f,0,13e-7f,6e-7f } ;
	//{ 0,0,0,0,0,0,0 } ;
	return t ;
        // clock cycle: 1/400 kHz <= rise + high + fall + low <= 1/10 kHz
    }
    
    Bang(Rpi::Peripheral *rpi,
	 Rpi::Pin      sclPin,
	 Rpi::Pin      sdaPin,
	 Addr            addr,
	 Timing<uint32_t> const &timing) 
	: rpi(rpi),sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing) {}

    enum class Line
    {
	Low, // pin connected to ground
	Off, // pin not connected (tri-state/open-drain)
    } ;
    
    struct Record
    {
	struct Reset
	{
	    uint32_t t0 ;
	    std::array<uint32_t,2> ackA ;
	} ;
	struct Read
	{
	    uint32_t t0 ;
	    std::array<uint32_t,3> ackA ;
	    using Byte = std::array<uint32_t,8> ;
	    std::array<Byte,2> byteA ;
	} ;
	struct Write
	{
	    uint32_t t0 ;
	    std::array<uint32_t,4> ackA ;
	} ;
    } ;

    uint16_t fetch(Record::Read const &record) const ;

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

    Error verify(Record::Reset const&) const ;
    Error verify(Record::Read  const&) const ;
    Error verify(Record::Write const&) const ;
    
    Record::Reset reset() ;
    Record::Read readConfig() ;
    Record::Read readSample() ;
    Record::Write writeConfig(uint16_t word) ;
    
private:

    Rpi::Peripheral *rpi ;
    
    Rpi::Pin sclPin,sdaPin ;

    Addr addr ;

    Timing<uint32_t> timing ;

    using Script = RpiExt::Bang::Enqueue ;

    void start(Script*) ;
    void stop (Script*,Line sda,uint32_t *t0) ;
    
    void recvBit(Script*,Line sda,uint32_t *t0,uint32_t *levels) ;
    void sendBit(Script*,Line from,Line to,uint32_t *t0) ;

    void recvByte(Script*,Line sda,uint32_t *t0,Record::Read::Byte*) ;
    void sendByte(Script*,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack) ;

    void reset(Script*,Record::Reset*) ;
    void read (Script*,Record::Read*,uint8_t reg) ;
    void write(Script*,Record::Write*,uint16_t data) ;
} ;

} }

#endif // INCLUDE_Device_Ads1115_Host_h
