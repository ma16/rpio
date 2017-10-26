// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Function_h_
#define _Rpi_Function_h_

// --------------------------------------------------------------------
// see BCM2835 ARM Peripherals: § 6.2: Alternative Function Assignments
//
// [todo] more functions described in
// www.raspberrypi.org/documentation/hardware/computemodule/RPI-CM-DATASHEET-V1_0.pdf
// --------------------------------------------------------------------

#include "GpioOld.h"
#include <vector>

namespace Rpi { namespace Function
{
  enum class Type : unsigned
  { 
    // 2x Broadcom Serial Controller (Master)
    Bsc0_Sda,Bsc0_Scl,Bsc1_Sda,Bsc1_Scl,
      
    // 1x Broadcom Serial Controller (Slave)
    BscS_Sda,BscS_Scl,
      
    // 3x General Purpose Clock
    Cp0,Cp1,Cp2,

    // 1x Serial Peripheral Interface ("Full")
    Spi0_Ce1,Spi0_Ce0,Spi0_Miso,Spi0_Mosi,Spi0_Sclk,

    // 2x Serial Peripheral Interface ("Lite")
    Spi1_Ce2,Spi1_Ce1,Spi1_Ce0,Spi1_Miso,Spi1_Mosi,Spi1_Sclk,
    Spi2_Ce2,Spi2_Ce1,Spi2_Ce0,Spi2_Miso,Spi2_Mosi,Spi2_Sclk,
  
    // 1x Serial Peripheral Interface (Slave)
    BscS_Mosi,BscS_Slck,BscS_Miso,BscS_Ce,
  
    // 2x Pulse Width Modulation
    Pwm0,Pwm1,

    // 2x Universal Asynchronous Receiver Transmitter
    Uart0_Txd,Uart0_Rxd,Uart0_Cts,Uart0_Rts,
    Uart1_Txd,Uart1_Rxd,Uart1_Cts,Uart1_Rts,

    // 1x Pulse Code Modulation
    Pcm_Clk,Pcm_Fs,Pcm_Din,Pcm_Dout,
  
    // 1x Secondary Memory
    Mem_Sa0,Mem_Sa1,Mem_Sa2,Mem_Sa3,Mem_Sa4,Mem_Sa5,
    Mem_Soe,Mem_Swe, 
    Mem_Sd0, Mem_Sd1, Mem_Sd2, Mem_Sd3, Mem_Sd4, Mem_Sd5, Mem_Sd6 ,Mem_Sd7, Mem_Sd8,
    Mem_Sd9,Mem_Sd10,Mem_Sd11,Mem_Sd12,Mem_Sd13,Mem_Sd14,Mem_Sd15,Mem_Sd16,Mem_Sd17,
  
    // 1x ARM
    Arm_Trst,Arm_Rtck,Arm_Tdo,Arm_Tck,Arm_Tdi,Arm_Tms,
  } ;

  using TypeN = Neat::Numerator<Type,Type::Arm_Tms> ;

  struct Record { Type type ; Pin pin ; GpioOld::Mode mode ; } ;

  char const* name(Type type) ;
  
  std::vector<Record> const& records() ;
  
} }

#endif // _Rpi_Function_h_
