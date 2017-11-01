// BSD 2-Clause License, see github.com/ma16/rpio

#include "Function.h"

Rpi::Gpio::Function::Type Rpi::Gpio::Function::get(Base gpio,Pin pin)
{
    static_assert(pin.max < 54,"") ;
    auto i = 0x0u + pin.value()/10u ;
    // ...10 pins per bank
    auto r = 3u * (pin.value()%10u) ;
    // ...3 bits for each pin
    auto w = gpio.ptr()[i] ;
    w >>= r ;
    return TypeEnum::coset(w).e() ;
}

void Rpi::Gpio::Function::set(Base gpio,Pin pin,Type mode)
{
    static_assert(pin.max < 54,"") ;
    auto i = 0x0u + pin.value()/10u ;
    // ...10 pins per bank
    auto r = 3u * (pin.value()%10u) ; 
    // ...3 bits for each pin
    auto w = gpio.ptr()[i] ;
    w &= ~(7u << r) ; 
    w |= TypeEnum(mode).n() << r ;
    gpio.ptr()[i] = w ;
    // [todo] read back and check if the same (race conditions)
}

std::vector<Rpi::Gpio::Function::Record> const& Rpi::Gpio::Function::records() 
{
    static std::vector<Record> recordV = {
  
	// 2x Broadcom Serial Controller (Master): (Sda,Scl)
  
	{ Device::Bsc0_Sda,Pin::make <0>(),Type::Alt0 },
	{ Device::Bsc0_Sda,Pin::make<28>(),Type::Alt0 },
	{ Device::Bsc0_Scl,Pin::make <1>(),Type::Alt0 },
	{ Device::Bsc0_Scl,Pin::make<29>(),Type::Alt0 },
  
	{ Device::Bsc1_Sda,Pin::make<2>(),Type::Alt0 },
	//{ Device::Bsc1_Sda,Pin::make<44>(),Type::Alt0 },
	{ Device::Bsc1_Scl,Pin::make<3>(),Type::Alt2 },
	//{ Device::Bsc1_Scl,Pin::make<45>(),Type::Alt2 },

	// 1x Broadcom Serial Controller (Slave): (Sda,Scl) 

	{ Device::BscS_Sda,Pin::make<18>(),Type::Alt3 },
	{ Device::BscS_Scl,Pin::make<19>(),Type::Alt3 },

	// 3x General Purpose Clock
  
	{ Device::Cp0,Pin::make <4>(),Type::Alt0 },
	{ Device::Cp0,Pin::make<20>(),Type::Alt5 },
	//{ Device::Cp0,Pin::make<32>(),Type::Alt0 },
	//{ Device::Cp0,Pin::make<34>(),Type::Alt0 },
  
	{ Device::Cp1,Pin::make <5>(),Type::Alt0 },
	{ Device::Cp0,Pin::make<21>(),Type::Alt5 },
	//{ Device::Cp1,Pin::make<42>(),Type::Alt0 },
	//{ Device::Cp1,Pin::make<44>(),Type::Alt0 },
  
	{ Device::Cp2,Pin::make<6>(),Type::Alt0 },
	//{ Device::Cp2,Pin::make<43>(),Type::Alt0 },

	// 1x Serial Peripheral Interface ("Full"): (Miso,Mosi,Clk,Ce0,Ce1)

	{ Device::Spi0_Ce1 ,Pin::make< 7>(),Type::Alt0 },
	//{ Device::Spi0_Ce1 ,Pin::make<35>(),Type::Alt0 },
	{ Device::Spi0_Ce0 ,Pin::make< 8>(),Type::Alt0 },
	//{ Device::Spi0_Ce0 ,Pin::make<36>(),Type::Alt0 },
	{ Device::Spi0_Miso,Pin::make< 9>(),Type::Alt0 },
	//{ Device::Spi0_Miso,Pin::make<37>(),Type::Alt0 },
	{ Device::Spi0_Mosi,Pin::make<10>(),Type::Alt0 },
	//{ Device::Spi0_Mosi,Pin::make<38>(),Type::Alt0 },
	{ Device::Spi0_Sclk,Pin::make<11>(),Type::Alt0 },
	//{ Device::Spi0_Sclk,Pin::make<39>(),Type::Alt0 },
  
	// 2x Serial Peripheral Interface ("Lite"): (Miso,Mosi,Clk,Ce0,Ce1,Ce2)

	{ Device::Spi1_Ce2 ,Pin::make<16>(),Type::Alt4 },
	{ Device::Spi1_Ce1 ,Pin::make<17>(),Type::Alt4 },
	{ Device::Spi1_Ce0 ,Pin::make<18>(),Type::Alt4 },
	{ Device::Spi1_Miso,Pin::make<19>(),Type::Alt4 },
	{ Device::Spi1_Mosi,Pin::make<20>(),Type::Alt4 },
	{ Device::Spi1_Sclk,Pin::make<21>(),Type::Alt4 },
    
	/*
	  { Device::Spi2_Ce2 ,Pin::make<45>(),Type::Alt4 },
	  { Device::Spi2_Ce1 ,Pin::make<44>(),Type::Alt4 },
	  { Device::Spi2_Ce0 ,Pin::make<43>(),Type::Alt4 },
	  { Device::Spi2_Miso,Pin::make<40>(),Type::Alt4 },
	  { Device::Spi2_Mosi,Pin::make<41>(),Type::Alt4 },
	  { Device::Spi2_Sclk,Pin::make<42>(),Type::Alt4 },
	*/
    
	// 1x Serial Peripheral Interface (Slave): (Mosi,Miso,Sclk,Ce)
    
	{ Device::BscS_Mosi,Pin::make<18>(),Type::Alt3 },
	{ Device::BscS_Slck,Pin::make<19>(),Type::Alt3 },
	{ Device::BscS_Miso,Pin::make<20>(),Type::Alt3 },
	{ Device::BscS_Ce  ,Pin::make<21>(),Type::Alt3 },
  
	// 2x Pulse Width Modulation
  
	{ Device::Pwm0,Pin::make<12>(),Type::Alt0 },
	{ Device::Pwm0,Pin::make<18>(),Type::Alt5 },
	//{ Device::Pwm0,Pin::make<40>(),Type::Alt0 },
  
	{ Device::Pwm1,Pin::make<13>(),Type::Alt0 },
	{ Device::Pwm0,Pin::make<19>(),Type::Alt5 },
	//{ Device::Pwm0,Pin::make<41>(),Type::Alt0 },
	//{ Device::Pwm0,Pin::make<45>(),Type::Alt0 },

	// 2x Universal Asynchronous Receiver Transmitter: (Tx,Rx,Cts,Rts)

	{ Device::Uart0_Txd,Pin::make<14>(),Type::Alt0 },
	{ Device::Uart0_Txd,Pin::make<30>(),Type::Alt3 },
	//{ Device::Uart0_Txd,Pin::make<36>(),Type::Alt2 },
	{ Device::Uart0_Rxd,Pin::make<15>(),Type::Alt0 },
	{ Device::Uart0_Rxd,Pin::make<31>(),Type::Alt3 },
	//{ Device::Uart0_Rxd,Pin::make<37>(),Type::Alt2 },
	{ Device::Uart0_Cts,Pin::make<16>(),Type::Alt3 },
	//{ Device::Uart0_Cts,Pin::make<32>(),Type::Alt3 },
	//{ Device::Uart0_Cts,Pin::make<38>(),Type::Alt2 },
	{ Device::Uart0_Rts,Pin::make<17>(),Type::Alt3 },
	//{ Device::Uart0_Rts,Pin::make<33>(),Type::Alt3 },
	//{ Device::Uart0_Rts,Pin::make<39>(),Type::Alt2 },

	{ Device::Uart1_Txd,Pin::make<14>(),Type::Alt5 },
	{ Device::Uart1_Txd,Pin::make<30>(),Type::Alt5 },
	//{ Device::Uart1_Txd,Pin::make<40>(),Type::Alt5 },
	{ Device::Uart1_Rxd,Pin::make<15>(),Type::Alt5 },
	{ Device::Uart1_Txd,Pin::make<31>(),Type::Alt5 },
	//{ Device::Uart1_Rxd,Pin::make<41>(),Type::Alt5 },
	{ Device::Uart1_Cts,Pin::make<16>(),Type::Alt5 },
	//{ Device::Uart1_Cts,Pin::make<32>(),Type::Alt5 }
	//{ Device::Uart1_Cts,Pin::make<42>(),Type::Alt5 },
	{ Device::Uart1_Rts,Pin::make<17>(),Type::Alt5 },
	//{ Device::Uart1_Rts,Pin::make<33>(),Type::Alt5 },
	//{ Device::Uart1_Rts,Pin::make<43>(),Type::Alt5 },

	// 1x Pulse Code Modulation: (Clk,Fs,Din,Dout)
  
	{ Device::Pcm_Clk ,Pin::make<18>(),Type::Alt0 },
	{ Device::Pcm_Clk ,Pin::make<28>(),Type::Alt2 },
	{ Device::Pcm_Fs  ,Pin::make<19>(),Type::Alt0 },
	{ Device::Pcm_Fs  ,Pin::make<29>(),Type::Alt2 },
	{ Device::Pcm_Din ,Pin::make<20>(),Type::Alt0 },
	{ Device::Pcm_Din ,Pin::make<30>(),Type::Alt2 },
	{ Device::Pcm_Dout,Pin::make<21>(),Type::Alt0 },
	{ Device::Pcm_Dout,Pin::make<31>(),Type::Alt2 },
  
	// 1x Secondary Memory: (Sa0,...,Sa5,Sd0,...,Sd17,Soe,Swe)
  
	{ Device::Mem_Sa0, Pin::make< 5>(),Type::Alt1 },
	{ Device::Mem_Sa0,Pin::make<28>(),Type::Alt1 },
	{ Device::Mem_Sa1, Pin::make< 4>(),Type::Alt1 },
	{ Device::Mem_Sa1,Pin::make<29>(),Type::Alt1 },
	{ Device::Mem_Sa2, Pin::make< 3>(),Type::Alt1 },
	{ Device::Mem_Sa2,Pin::make<30>(),Type::Alt1 },
	{ Device::Mem_Sa3, Pin::make< 2>(),Type::Alt1 },
	{ Device::Mem_Sa3,Pin::make<31>(),Type::Alt1 },
	{ Device::Mem_Sa4, Pin::make< 1>(),Type::Alt1 },
	/*{ Device::Mem_Sa4,Pin::make<32>(),Type::Alt1 },*/
	{ Device::Mem_Sa5, Pin::make< 0>(),Type::Alt1 },
	/*{ Device::Mem_Sa5,Pin::make<33>(),Type::Alt1 },*/
	{ Device::Mem_Soe, Pin::make< 6>(),Type::Alt1 },
	/*{ Device::Mem_Soe,Pin::make<34>(),Type::Alt1 },*/
	{ Device::Mem_Swe, Pin::make< 7>(),Type::Alt1 },
	/*{ Device::Mem_Soe,Pin::make<35>(),Type::Alt1 },*/
	{ Device::Mem_Sd0, Pin::make< 8>(),Type::Alt1 },
	/*{ Device::Mem_Sd0,Pin::make<36>(),Type::Alt1 },*/
	{ Device::Mem_Sd1, Pin::make< 9>(),Type::Alt1 },
	/*{ Device::Mem_Sd1,Pin::make<37>(),Type::Alt1 },*/
	{ Device::Mem_Sd2, Pin::make<10>(),Type::Alt1 },
	/*{ Device::Mem_Sd2,Pin::make<38>(),Type::Alt1 },*/
	{ Device::Mem_Sd3, Pin::make<11>(),Type::Alt1 },
	/*{ Device::Mem_Sd3,Pin::make<39>(),Type::Alt1 },*/
	{ Device::Mem_Sd4, Pin::make<12>(),Type::Alt1 },
	/*{ Device::Mem_Sd4,Pin::make<40>(),Type::Alt1 },*/
	{ Device::Mem_Sd5, Pin::make<13>(),Type::Alt1 },
	/*{ Device::Mem_Sd5,Pin::make<41>(),Type::Alt1 },*/
	{ Device::Mem_Sd6, Pin::make<14>(),Type::Alt1 },
	/*{ Device::Mem_Sd6,Pin::make<42>(),Type::Alt1 },*/
	{ Device::Mem_Sd7, Pin::make<15>(),Type::Alt1 },
	/*{ Device::Mem_Sd7,Pin::make<43>(),Type::Alt1 },*/
	{ Device::Mem_Sd8, Pin::make<16>(),Type::Alt1 },
	{ Device::Mem_Sd9, Pin::make<17>(),Type::Alt1 },
	{ Device::Mem_Sd10,Pin::make<18>(),Type::Alt1 },
	{ Device::Mem_Sd11,Pin::make<19>(),Type::Alt1 },
	{ Device::Mem_Sd12,Pin::make<20>(),Type::Alt1 },
	{ Device::Mem_Sd13,Pin::make<21>(),Type::Alt1 },
	{ Device::Mem_Sd14,Pin::make<22>(),Type::Alt1 },
	{ Device::Mem_Sd15,Pin::make<23>(),Type::Alt1 },
	{ Device::Mem_Sd16,Pin::make<24>(),Type::Alt1 },
	{ Device::Mem_Sd17,Pin::make<15>(),Type::Alt1 },
  
	// 1x ARM: (Trst,Rtck,Tdo,Tck,Tdi,Tms)
  
	{ Device::Arm_Trst,Pin::make<22>(),Type::Alt4 },
	{ Device::Arm_Rtck,Pin::make<23>(),Type::Alt4 },
	{ Device::Arm_Rtck,Pin::make< 6>(),Type::Alt5 },
	{ Device::Arm_Tdo ,Pin::make<24>(),Type::Alt4 },
	{ Device::Arm_Tdo ,Pin::make< 5>(),Type::Alt5 },
	{ Device::Arm_Tck ,Pin::make<25>(),Type::Alt4 },
	{ Device::Arm_Tck ,Pin::make<13>(),Type::Alt5 },
	{ Device::Arm_Tdi ,Pin::make<26>(),Type::Alt4 },
	{ Device::Arm_Tdi ,Pin::make< 4>(),Type::Alt5 },
	{ Device::Arm_Tms ,Pin::make<27>(),Type::Alt4 },
	{ Device::Arm_Tms ,Pin::make<12>(),Type::Alt5 },

	// [todo] (Sd1_Clk,Sd1_Cmd,Sd1_Dat0,...,Sd1_dat3)
    } ;
  
    return recordV ;
}

char const* Rpi::Gpio::Function::name(Device device) 
{
    static char const* v[] =
    {
	"Bsc0_Sda","Bsc0_Scl",
	"Bsc1_Sda","Bsc1_Scl",
	"BscS_Sda","BscS_Scl",
	"Cp0",
	"Cp1",
	"Cp2",
	"Spi0_Ce1","Spi0_Ce0","Spi0_Miso","Spi0_Mosi","Spi0_Sclk",
	"Spi1_Ce2","Spi1_Ce1","Spi1_Ce0","Spi1_Miso","Spi1_Mosi","Spi1_Sclk",
	"Spi2_Ce2","Spi2_Ce1","Spi2_Ce0","Spi2_Miso","Spi2_Mosi","Spi2_Sclk",
	"BscS_Mosi","BscS_Slck","BscS_Miso","BscS_Ce",
	"Pwm0",
	"Pwm1",
	"Uart0_Txd","Uart0_Rxd","Uart0_Cts","Uart0_Rts",
	"Uart1_Txd","Uart1_Rxd","Uart1_Cts","Uart1_Rts",
	"Pcm_Clk", "Pcm_Fs","Pcm_Din","Pcm_Dout",
	"Mem_Sa0", "Mem_Sa1", "Mem_Sa2","Mem_Sa3","Mem_Sa4","Mem_Sa5",
	"Mem_Soe", "Mem_Swe",
	"Mem_Sd0", "Mem_Sd1", "Mem_Sd2", "Mem_Sd3",
	"Mem_Sd4", "Mem_Sd5", "Mem_Sd6", "Mem_Sd7",
	"Mem_Sd8", "Mem_Sd9", "Mem_Sd10","Mem_Sd11",
	"Mem_Sd12","Mem_Sd13","Mem_Sd14","Mem_Sd15",
	"Mem_Sd16","Mem_Sd17",
	"Arm_Trst","Arm_Rtck","Arm_Tdo","Arm_Tck","Arm_Tdi","Arm_Tms",
    } ;
    static_assert(DeviceEnum::max == sizeof(v)/sizeof(v[0])-1,"") ;
    return v[DeviceEnum(device).n()] ;
}
