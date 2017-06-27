// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Device_Max7219_Host_h_
#define _Console_Device_Max7219_Host_h_

#include <Rpi/Gpio.h>

namespace Console { namespace Device { namespace Max7219 { struct Host
{
  void send(uint16_t data) ;

  void latch() ;
    
  Host(Rpi::Gpio gpio,Rpi::Pin loadPin,Rpi::Pin clkPin,Rpi::Pin datPin) ;

private:
  
  Rpi::Gpio gpio ; Rpi::Pin loadPin ; Rpi::Pin clkPin ; Rpi::Pin datPin ;

  static void hold() ;
  
} ; } } }

#endif // _Console_Device_Max7219_Host_h_
