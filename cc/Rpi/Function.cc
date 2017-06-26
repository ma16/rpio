// BSD 2-Clause License, see github.com/ma16/rpio

#include "Function.h"

std::vector<Rpi::Function::Record> const& Rpi::Function::records() 
{
  using Mode = Gpio::Mode ;
  
  static std::vector<Record> recordV = {
  
    // 2x Broadcom Serial Controller (Master): (Sda,Scl)
  
    { Type::Bsc0_Sda,Pin::make<0>(),Mode::Alt0 },{ Type::Bsc0_Sda,Pin::make<28>(),Mode::Alt0 },
    { Type::Bsc0_Scl,Pin::make<1>(),Mode::Alt0 },{ Type::Bsc0_Scl,Pin::make<29>(),Mode::Alt0 },
  
    { Type::Bsc1_Sda,Pin::make<2>(),Mode::Alt0 },/*{ Type::Bsc1_Sda,Pin::make<44>(),Mode::Alt0 },*/
    { Type::Bsc1_Scl,Pin::make<3>(),Mode::Alt2 },/*{ Type::Bsc1_Scl,Pin::make<45>(),Mode::Alt2 },*/

    // 1x Broadcom Serial Controller (Slave): (Sda,Scl) 

    { Type::BscS_Sda,Pin::make<18>(),Mode::Alt3 },
    { Type::BscS_Scl,Pin::make<19>(),Mode::Alt3 },

    // 3x General Purpose Clock
  
    { Type::Cp0,Pin::make<4>(),Mode::Alt0 },{ Type::Cp0,Pin::make<20>(),Mode::Alt5 },/*{ Type::Cp0,Pin::make<32>(),Mode::Alt0 },{ Type::Cp0,Pin::make<34>(),Mode::Alt0 },*/
  
    { Type::Cp1,Pin::make<5>(),Mode::Alt0 },{ Type::Cp0,Pin::make<21>(),Mode::Alt5 },/*{ Type::Cp1,Pin::make<42>(),Mode::Alt0 },{ Type::Cp1,Pin::make<44>(),Mode::Alt0 },*/
  
    { Type::Cp2,Pin::make<6>(),Mode::Alt0 },/*{ Type::Cp2,Pin::make<43>(),Mode::Alt0 },*/

    // 1x Serial Peripheral Interface ("Full"): (Miso,Mosi,Clk,Ce0,Ce1)

    { Type::Spi0_Ce1 ,Pin::make< 7>(),Mode::Alt0 },/*{ Type::Spi0_Ce1 ,Pin::make<35>(),Mode::Alt0 },*/
    { Type::Spi0_Ce0 ,Pin::make< 8>(),Mode::Alt0 },/*{ Type::Spi0_Ce0 ,Pin::make<36>(),Mode::Alt0 },*/
    { Type::Spi0_Miso,Pin::make< 9>(),Mode::Alt0 },/*{ Type::Spi0_Miso,Pin::make<37>(),Mode::Alt0 },*/
    { Type::Spi0_Mosi,Pin::make<10>(),Mode::Alt0 },/*{ Type::Spi0_Mosi,Pin::make<38>(),Mode::Alt0 },*/
    { Type::Spi0_Sclk,Pin::make<11>(),Mode::Alt0 },/*{ Type::Spi0_Sclk,Pin::make<39>(),Mode::Alt0 },*/
  
    // 2x Serial Peripheral Interface ("Lite"): (Miso,Mosi,Clk,Ce0,Ce1,Ce2)

    { Type::Spi1_Ce2 ,Pin::make<16>(),Mode::Alt4 },
    { Type::Spi1_Ce1 ,Pin::make<17>(),Mode::Alt4 },
    { Type::Spi1_Ce0 ,Pin::make<18>(),Mode::Alt4 },
    { Type::Spi1_Miso,Pin::make<19>(),Mode::Alt4 },
    { Type::Spi1_Mosi,Pin::make<20>(),Mode::Alt4 },
    { Type::Spi1_Sclk,Pin::make<21>(),Mode::Alt4 },
    
    /*
    { Type::Spi2_Ce2 ,Pin::make<45>(),Mode::Alt4 },
    { Type::Spi2_Ce1 ,Pin::make<44>(),Mode::Alt4 },
    { Type::Spi2_Ce0 ,Pin::make<43>(),Mode::Alt4 },
    { Type::Spi2_Miso,Pin::make<40>(),Mode::Alt4 },
    { Type::Spi2_Mosi,Pin::make<41>(),Mode::Alt4 },
    { Type::Spi2_Sclk,Pin::make<42>(),Mode::Alt4 },
    */
    
    // 1x Serial Peripheral Interface (Slave): (Mosi,Miso,Sclk,Ce)
    
    { Type::BscS_Mosi,Pin::make<18>(),Mode::Alt3 },
    { Type::BscS_Slck,Pin::make<19>(),Mode::Alt3 },
    { Type::BscS_Miso,Pin::make<20>(),Mode::Alt3 },
    { Type::BscS_Ce  ,Pin::make<21>(),Mode::Alt3 },
  
    // 2x Pulse Width Modulation
  
    { Type::Pwm0,Pin::make<12>(),Mode::Alt0 },{ Type::Pwm0,Pin::make<18>(),Mode::Alt5 },/*{ Type::Pwm0,Pin::make<40>(),Mode::Alt0 },*/
  
    { Type::Pwm1,Pin::make<13>(),Mode::Alt0 },{ Type::Pwm0,Pin::make<19>(),Mode::Alt5 },/*{ Type::Pwm0,Pin::make<41>(),Mode::Alt0 },{ Type::Pwm0,Pin::make<45>(),Mode::Alt0 },*/

    // 2x Universal Asynchronous Receiver Transmitter: (Tx,Rx,Cts,Rts)

    { Type::Uart0_Txd,Pin::make<14>(),Mode::Alt0 },{ Type::Uart0_Txd,Pin::make<30>(),Mode::Alt3 },/*{ Type::Uart0_Txd,Pin::make<36>(),Mode::Alt2 },*/
    { Type::Uart0_Rxd,Pin::make<15>(),Mode::Alt0 },{ Type::Uart0_Rxd,Pin::make<31>(),Mode::Alt3 },/*{ Type::Uart0_Rxd,Pin::make<37>(),Mode::Alt2 },*/
    { Type::Uart0_Cts,Pin::make<16>(),Mode::Alt3 },/*{ Type::Uart0_Cts,Pin::make<32>(),Mode::Alt3 },{ Type::Uart0_Cts,Pin::make<38>(),Mode::Alt2 },*/
    { Type::Uart0_Rts,Pin::make<17>(),Mode::Alt3 },/*{ Type::Uart0_Rts,Pin::make<33>(),Mode::Alt3 },{ Type::Uart0_Rts,Pin::make<39>(),Mode::Alt2 },*/

    { Type::Uart1_Txd,Pin::make<14>(),Mode::Alt5 },{ Type::Uart1_Txd,Pin::make<30>(),Mode::Alt5 },/*{ Type::Uart1_Txd,Pin::make<40>(),Mode::Alt5 },*/
    { Type::Uart1_Rxd,Pin::make<15>(),Mode::Alt5 },{ Type::Uart1_Txd,Pin::make<31>(),Mode::Alt5 },/*{ Type::Uart1_Rxd,Pin::make<41>(),Mode::Alt5 },*/
    { Type::Uart1_Cts,Pin::make<16>(),Mode::Alt5 },/*{ Type::Uart1_Cts,Pin::make<32>(),Mode::Alt5 },{ Type::Uart1_Cts,Pin::make<42>(),Mode::Alt5 },*/
    { Type::Uart1_Rts,Pin::make<17>(),Mode::Alt5 },/*{ Type::Uart1_Rts,Pin::make<33>(),Mode::Alt5 },{ Type::Uart1_Rts,Pin::make<43>(),Mode::Alt5 },*/

    // 1x Pulse Code Modulation: (Clk,Fs,Din,Dout)
  
    { Type::Pcm_Clk ,Pin::make<18>(),Mode::Alt0 },{ Type::Pcm_Clk ,Pin::make<28>(),Mode::Alt2 },
    { Type::Pcm_Fs  ,Pin::make<19>(),Mode::Alt0 },{ Type::Pcm_Fs  ,Pin::make<29>(),Mode::Alt2 },
    { Type::Pcm_Din ,Pin::make<20>(),Mode::Alt0 },{ Type::Pcm_Din ,Pin::make<30>(),Mode::Alt2 },
    { Type::Pcm_Dout,Pin::make<21>(),Mode::Alt0 },{ Type::Pcm_Dout,Pin::make<31>(),Mode::Alt2 },
  
    // 1x Secondary Memory: (Sa0,...,Sa5,Sd0,...,Sd17,Soe,Swe)
  
    { Type::Mem_Sa0, Pin::make< 5>(),Mode::Alt1 },{ Type::Mem_Sa0,Pin::make<28>(),Mode::Alt1 },
    { Type::Mem_Sa1, Pin::make< 4>(),Mode::Alt1 },{ Type::Mem_Sa1,Pin::make<29>(),Mode::Alt1 },
    { Type::Mem_Sa2, Pin::make< 3>(),Mode::Alt1 },{ Type::Mem_Sa2,Pin::make<30>(),Mode::Alt1 },
    { Type::Mem_Sa3, Pin::make< 2>(),Mode::Alt1 },{ Type::Mem_Sa3,Pin::make<31>(),Mode::Alt1 },
    { Type::Mem_Sa4, Pin::make< 1>(),Mode::Alt1 },/*{ Type::Mem_Sa4,Pin::make<32>(),Mode::Alt1 },*/
    { Type::Mem_Sa5, Pin::make< 0>(),Mode::Alt1 },/*{ Type::Mem_Sa5,Pin::make<33>(),Mode::Alt1 },*/
    { Type::Mem_Soe, Pin::make< 6>(),Mode::Alt1 },/*{ Type::Mem_Soe,Pin::make<34>(),Mode::Alt1 },*/
    { Type::Mem_Swe, Pin::make< 7>(),Mode::Alt1 },/*{ Type::Mem_Soe,Pin::make<35>(),Mode::Alt1 },*/
    { Type::Mem_Sd0, Pin::make< 8>(),Mode::Alt1 },/*{ Type::Mem_Sd0,Pin::make<36>(),Mode::Alt1 },*/
    { Type::Mem_Sd1, Pin::make< 9>(),Mode::Alt1 },/*{ Type::Mem_Sd1,Pin::make<37>(),Mode::Alt1 },*/
    { Type::Mem_Sd2, Pin::make<10>(),Mode::Alt1 },/*{ Type::Mem_Sd2,Pin::make<38>(),Mode::Alt1 },*/
    { Type::Mem_Sd3, Pin::make<11>(),Mode::Alt1 },/*{ Type::Mem_Sd3,Pin::make<39>(),Mode::Alt1 },*/
    { Type::Mem_Sd4, Pin::make<12>(),Mode::Alt1 },/*{ Type::Mem_Sd4,Pin::make<40>(),Mode::Alt1 },*/
    { Type::Mem_Sd5, Pin::make<13>(),Mode::Alt1 },/*{ Type::Mem_Sd5,Pin::make<41>(),Mode::Alt1 },*/
    { Type::Mem_Sd6, Pin::make<14>(),Mode::Alt1 },/*{ Type::Mem_Sd6,Pin::make<42>(),Mode::Alt1 },*/
    { Type::Mem_Sd7, Pin::make<15>(),Mode::Alt1 },/*{ Type::Mem_Sd7,Pin::make<43>(),Mode::Alt1 },*/
    { Type::Mem_Sd8, Pin::make<16>(),Mode::Alt1 },
    { Type::Mem_Sd9, Pin::make<17>(),Mode::Alt1 },
    { Type::Mem_Sd10,Pin::make<18>(),Mode::Alt1 },
    { Type::Mem_Sd11,Pin::make<19>(),Mode::Alt1 },
    { Type::Mem_Sd12,Pin::make<20>(),Mode::Alt1 },
    { Type::Mem_Sd13,Pin::make<21>(),Mode::Alt1 },
    { Type::Mem_Sd14,Pin::make<22>(),Mode::Alt1 },
    { Type::Mem_Sd15,Pin::make<23>(),Mode::Alt1 },
    { Type::Mem_Sd16,Pin::make<24>(),Mode::Alt1 },
    { Type::Mem_Sd17,Pin::make<15>(),Mode::Alt1 },
  
    // 1x ARM: (Trst,Rtck,Tdo,Tck,Tdi,Tms)
  
    { Type::Arm_Trst,Pin::make<22>(),Mode::Alt4 },
    { Type::Arm_Rtck,Pin::make<23>(),Mode::Alt4 },{ Type::Arm_Rtck,Pin::make< 6>(),Mode::Alt5 },
    { Type::Arm_Tdo ,Pin::make<24>(),Mode::Alt4 },{ Type::Arm_Tdo ,Pin::make< 5>(),Mode::Alt5 },
    { Type::Arm_Tck ,Pin::make<25>(),Mode::Alt4 },{ Type::Arm_Tck ,Pin::make<13>(),Mode::Alt5 },
    { Type::Arm_Tdi ,Pin::make<26>(),Mode::Alt4 },{ Type::Arm_Tdi ,Pin::make< 4>(),Mode::Alt5 },
    { Type::Arm_Tms ,Pin::make<27>(),Mode::Alt4 },{ Type::Arm_Tms ,Pin::make<12>(),Mode::Alt5 },

    // [todo] (Sd1_Clk,Sd1_Cmd,Sd1_Dat0,...,Sd1_dat3)
  } ;
  
  return recordV ;
}

char const* Rpi::Function::name(Type type) 
{
  static char const* nameV[] = {
    "Bsc0_Sda","Bsc0_Scl","Bsc1_Sda","Bsc1_Scl",
    "BscS_Sda","BscS_Scl",
    "Cp0","Cp1","Cp2",
    "Spi0_Ce1","Spi0_Ce0","Spi0_Miso","Spi0_Mosi","Spi0_Sclk",
    "Spi1_Ce2","Spi1_Ce1","Spi1_Ce0","Spi1_Miso","Spi1_Mosi","Spi1_Sclk",
    "Spi2_Ce2","Spi2_Ce1","Spi2_Ce0","Spi2_Miso","Spi2_Mosi","Spi2_Sclk",
    "BscS_Mosi","BscS_Slck","BscS_Miso","BscS_Ce",
    "Pwm0","Pwm1",
    "Uart0_Txd","Uart0_Rxd","Uart0_Cts","Uart0_Rts",
    "Uart1_Txd","Uart1_Rxd","Uart1_Cts","Uart1_Rts",
    "Pcm_Clk","Pcm_Fs","Pcm_Din","Pcm_Dout",
    "Mem_Sa0","Mem_Sa1","Mem_Sa2","Mem_Sa3","Mem_Sa4","Mem_Sa5",
    "Mem_Soe","Mem_Swe",
    "Mem_Sd0", "Mem_Sd1", "Mem_Sd2", "Mem_Sd3", "Mem_Sd4", "Mem_Sd5", "Mem_Sd6", "Mem_Sd7", "Mem_Sd8",
    "Mem_Sd9","Mem_Sd10","Mem_Sd11","Mem_Sd12","Mem_Sd13","Mem_Sd14","Mem_Sd15","Mem_Sd16","Mem_Sd17",
    "Arm_Trst","Arm_Rtck","Arm_Tdo","Arm_Tck","Arm_Tdi","Arm_Tms",
  } ;
  static_assert(TypeN::max == sizeof(nameV)/sizeof(nameV[0])-1,"") ;
  return nameV[TypeN(type).n()] ;
}
