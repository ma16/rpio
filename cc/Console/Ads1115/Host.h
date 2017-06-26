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

#ifndef _Console_Ads1115_Host_h_
#define _Console_Ads1115_Host_h_

#include <Neat/Enum.h>
#include <Rpi/Counter.h>
#include <Rpi/Gpio.h>
#include <boost/optional.hpp>

namespace Console { namespace Ads1115 { struct Host
{
  // (Single=1/Diff=0,D2,D1,D0)
  using source_t = Neat::Enum<unsigned,0x7> ;
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

  using sample_t = int16_t ;
  // ...sampled digitized (signed!) value

  struct Timing // nano seconds
  {
    unsigned buf   =  600 ; // Bus free time between START and STOP condition
    unsigned hdsta =  600 ; // Hold time after repeated START condition.
    unsigned susto =  600 ; // Stop condition setup time
    
    unsigned sudat =  100 ; // Data setup time
    unsigned low   = 1300 ; // SCL clock low period
    unsigned high  =  600 ; // SCL clock high period

    // [todo] verify that SCL period >= 10 kHz (<=100us)
    // [todo] what is the maximum delay we can expect an ack and how long is the ack?
  } ;

  enum class error_t {
    NoAck = 0,
  } ;

  boost::optional<error_t> error() { return this->error_ ; }
  
  using addr_t = Neat::Enum<unsigned,0x7f> ;

  Host(Rpi::Peripheral *rpi,Rpi::Pin sclPin,Rpi::Pin sdaPin,addr_t addr,Timing const &t,bool monitor) ;

  bool doReset() ;
  
  boost::optional<uint16_t> readConfig() { return read(1) ; }
  boost::optional<uint16_t> readSample() { return read(0) ; }
  
  bool writeConfig(uint16_t word) { return write(1,word) ; }
  
private:
  
  Rpi::Counter counter ; Rpi::Gpio gpio ; Rpi::Pin sclPin,sdaPin ; addr_t addr ; Timing const &timing ; bool monitor ;

  void start() ; void stop() ;
  
  void sclHi() ; void sclLo() ;
  
  void sdaLo() ; bool sdaLevel() ; void sdaOff() ;
  
  void sendAck() {            sendBit(false) ; }  void sendBit(bool) ; void    sendByte(uint8_t) ; 
  bool recvAck() { return false == recvBit() ; }  bool recvBit(    ) ; uint8_t recvByte(       ) ;

  unsigned hold(unsigned delay) ;
  
  boost::optional<uint16_t> read(uint8_t rix) ;
  
  bool write(uint8_t rix,uint16_t data) ;

  boost::optional<error_t> error_ ; boost::none_t set(error_t error) { error_ = error ; return boost::none ; }
    
} ; } }

#endif // _Console_Ads1115_Host_h_
