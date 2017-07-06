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

	/* notes:
	   - susta (repeated START condition setup time) is the same
	     as buf time
	   - there are also maximum fall and raise time constraints for
	     the SCL edges (300ns) which we could verify, however, not 
	     at this moment.
	   - the implementation doesn't verify the clock cycle, which 
	     should be: 1/400 kHz <= (high + low) <= 1/10 kHz
	*/
	
	Timing(T buf,T hdsta,T susto,T sudat,T hddat,T low,T high) :
	    buf    (buf), hdsta(hdsta), susto(susto), sudat(sudat),
	    hddat(hddat), low    (low), high  (high) {}
    } ;

    static Timing<float> const& fast_timing()
    {
	// values from datasheet (in seconds):
	static Timing<float> t =
	{ 6e-7f,6e-7f,6e-7f,1e-7f,0,13e-7f,6e-7f } ;
	return t ;
    }
    
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
    int16_t sample(Record::Read const &record) const
    { return static_cast<int16_t>(this->fetch(record)) ; }

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

    struct Generator
    {
	Generator(
	    Rpi::Pin sclPin,
	    Rpi::Pin sdaPin,
	    Addr addr,
	    Timing<uint32_t> const &timing) 
	    :
	    sclPin(sclPin),sdaPin(sdaPin),addr(addr),timing(timing) {}

	using Script = std::vector<RpiExt::Bang::Command> ;
	
	Script reset      (Record::Reset*) ;
	Script readConfig (Record::Read*) ;
	Script readSample (Record::Read*) ;
	Script writeConfig(Record::Write*,uint16_t word) ;
	
    private:

	friend Bang ;
	
	Rpi::Pin sclPin,sdaPin ;

	Addr addr ;

	Timing<uint32_t> timing ;

	using Draft = RpiExt::Bang::Enqueue ;

	void start(Draft*) ;
	void stop (Draft*,Line sda,uint32_t *t0) ;
    
	void recvBit(Draft*,Line sda,uint32_t *t0,uint32_t *levels) ;
	void sendBit(Draft*,Line from,Line to,uint32_t *t0) ;

	void recvByte(Draft*,Line sda,uint32_t *t0,Record::Read::Byte*) ;
	void sendByte(Draft*,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack) ;
    
	void read(Draft*,Record::Read*,uint8_t reg) ;
	
	void reset      (Draft*,Record::Reset*) ;
	void readConfig (Draft*,Record::Read*) ;
	void readSample (Draft*,Record::Read*) ;
	void writeConfig(Draft*,Record::Write*,uint16_t word) ;
    } ;
    
    Record::Reset reset() ;
    Record::Read readConfig() ;
    Record::Read readSample() ;
    Record::Write writeConfig(uint16_t word) ;
    
//private:

    Bang(Rpi::Peripheral *rpi,Generator const &gen) : rpi(rpi),gen(gen) {}

    Rpi::Peripheral *rpi ;
    Generator gen ;
    
} ;

} }

#endif // INCLUDE_Device_Ads1115_Host_h
