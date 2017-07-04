// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// The ADS1115 is an I2C driven 15-bit (+sign) ADC with 2x2 channels
// and a maximum sampling rate of somewhat below 1k/s.
//
// This implementation does just support its "single-shot" mode with
// an up to 400khz bus clock
//
// This implementation does _not_ support:
// --continuous mode
// --comparator configuration
// --high speed I2C bus clock
//
// The state of the host is defined by the two pins Scl and Sda:
//
// SCL : Lo | Hi   [todo] run SCL also with Lo/Off
// SDA : Lo | Off
// 
// Please refer to the ADS1115 data sheet for more details.
// --------------------------------------------------------------------

#ifndef INCLUDE_Device_Ads1115_Host_h
#define INCLUDE_Device_Ads1115_Host_h

#include <Neat/Enum.h>
#include <RpiExt/Bang.h>
#include <boost/optional.hpp>

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
    // ...signed digital sample value

    template<typename T> struct Timing 
    {
	T buf   ; // (min) bus free time between START and STOP condition
	T hdsta ; // (min) hold time after repeated START condition.
	T susto ; // (min) stop condition setup time
	T sudat ; // (min) data setup time
	T hddat ; // (min) data hold time
	T low   ; // (min) SCL clock low period
	T high  ; // (min) SCL clock high period
	T fall  ; // (max) clock/data fall time
	T rise  ; // (max) clock/data rise time

	Timing(
	    T buf,   T hdsta, T susto, 
	    T sudat, T hddat, T low,   
	    T high,  T fall,  T rise)
	    :
	    buf    (buf), hdsta(hdsta), susto(susto),
	    sudat(sudat), hddat(hddat), low    (low),
	    high  (high), fall  (fall), rise (rise) {}


        // clock cycle: 1/400 kHz <= rise + high + fall + low <= 1/10 kHz

	// [todo] verify that SCL period >= 10 kHz (<=100us)
	// [todo] what is the maximum delay we can expect an ack and how long is the ack?
    } ;

    static Timing<float> const& default_timing()
    {
	static Timing<float> t =
	{ 6e-7f,6e-7f,6e-7f,1e-7f,0,13e-7f,6e-7f,3e-7f,3e-7f } ;
	return t ;
    }
    
    enum class Error
    {
	NoAck = 0,
    } ;

    boost::optional<Error> error() { return this->error_ ; }
  
    using Addr = Neat::Enum<unsigned,0x7f> ;

    Bang(Rpi::Peripheral *rpi,
	 Rpi::Pin sclPin,
	 Rpi::Pin sdaPin,
	 Addr addr,
	 Timing<uint32_t> const &timing,
	 bool monitor) ;

    bool doReset() ;
  
    boost::optional<uint16_t> readSample() { return read(0) ; }
  


    // start over:

    struct Record
    {
	// set configuration
	uint32_t t0 ;
	uint32_t ack[4] ;
	uint32_t recv[2][8] ;
    } ;

    boost::optional<uint16_t> readConfig() { return read(1) ; }
    Record readConfig2() ;
    
    bool writeConfig(uint16_t word) { return write(1,word) ; }
    bool writeConfig2(uint16_t word) ;
    
    enum class Line { Low,Off } ;
    
    using Script = std::vector<RpiExt::Bang::Command> ;

    Script makeWriteScript(uint16_t data,Record *record) ;
    Script makeReadScript(Record *record) ;

    void start(RpiExt::Bang::Enqueue *q) ;
    void stop(RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0) ;

    
    void recvBit(RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0,uint32_t *levels) ;
    void sendBit(RpiExt::Bang::Enqueue *q,Line from,Line to,uint32_t *t0) ;
    

    void recvByte(RpiExt::Bang::Enqueue *q,Line sda,uint32_t *t0,uint32_t (*levels)[8]) ;
    void sendByte(RpiExt::Bang::Enqueue *q,Line sda,uint8_t byte,uint32_t *t0,uint32_t *ack) ;

    
    
private:

    Rpi::Peripheral *rpi ;
    
    Rpi::Counter counter ;

    Rpi::Gpio gpio ;

    Rpi::Pin sclPin,sdaPin ;

    Addr addr ;

    Timing<uint32_t> timing ;

    bool monitor ;

    void start() ; void stop() ;
  
    void sclHi() ; void sclLo() ;
  
    void sdaLo() ; bool sdaLevel() ; void sdaOff() ;
  
    void sendAck() {            sendBit(false) ; }  void sendBit(bool) ; void    sendByte(uint8_t) ; 
    bool recvAck() { return false == recvBit() ; }  bool recvBit(    ) ; uint8_t recvByte(       ) ;

    unsigned hold(unsigned delay) ;
  
    boost::optional<uint16_t> read(uint8_t rix) ;
  
    bool write(uint8_t rix,uint16_t data) ;

    boost::optional<Error> error_ ; boost::none_t set(Error error) { error_ = error ; return boost::none ; }
    
} ;

} }

#endif // INCLUDE_Device_Ads1115_Host_h
