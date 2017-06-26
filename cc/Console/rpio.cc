// BSD 2-Clause License, see github.com/ma16/rpio

#include "rpio.h"
#include <Ui/strto.h>
#include <iostream>

static Posix::Fd::uoff_t base_addr(Ui::ArgL *argL)
{
  auto addr = argL->option("--base") ;
  if (addr)
    return Ui::strto<Posix::Fd::uoff_t>(*addr) ;
  if (argL->pop_if("--devtree"))
    return Rpi::Peripheral::by_devtree() ;
  return Rpi::Peripheral::by_cpuinfo() ;
}

int main(int argc,char **argv)
{
  try {
    auto argL = Ui::ArgL::make(argc-1,argv+1) ;

    if (argL.empty() || argL.peek() == "help") { 
      std::cout << "arguments: [BASE] MODE [help]\n"
		<< '\n'
		<< "BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000\n"
		<< "     | --devtree       # use info in /proc/device-tree/soc/ranges\n"
		<< '\n'
		<< "If BASE is not given then the peripheral address is derived from\n"
		<< "the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.\n"
		<< '\n'
		<< "MODE : ads1115     # ADC control for the ADS1115 circuit\n"
		<< "     | bang        # generic bit-banging\n"
		<< "     | clock       # r/w clock sources\n"
		<< "     | cp          # clock-pulse generator (GPIO Clock)\n"
		<< "     | dma         # DMA setup and tests\n"
		<< "     | gpio        # r/w GPIO\n"
		<< "     | intr        # enable/disable ARM and GPU interrupts\n"
		<< "     | max7219     # dot-matrix control for the MAX7219 circuit\n"
		<< "     | mbox        # Mailbox access\n"
		<< "     | mcp3008     # ADC control for the MCP3008 circuit\n"
		<< "     | poke        # r/w any word in peripheral address space\n"
		<< "     | pwm         # PWM setup\n"
		<< "     | sample      # sample data\n"
		<< "     | shm         # shared memory control (POSIX IPC)\n"
		<< "     | spi0        # serial peripheral interface (controller #0)\n"
		<< "     | spi1        # serial peripheral interface (controller #1)\n"
		<< "     | spi-slave   # serial peripheral interface (slave)\n"
		<< "     | throughput  # i/o and memory performance tests\n"
		<< "     | ws2812b     # Serial interface for WS2812B (LED strip)\n"
		<< '\n'
		<< "Use the keyword help for additional information.\n"
		<< std::flush ;
      return 0 ;
    }
    
    auto rpi = Rpi::Peripheral::make(base_addr(&argL)) ;
    std::string arg = argL.pop() ;
  
    if      (arg ==    "ads1115") Console::   Ads1115::invoke(rpi.get(),&argL) ;
    else if (arg ==       "bang") Console::      Bang::invoke(rpi.get(),&argL) ;
    else if (arg ==      "clock") Console::     Clock::invoke(rpi.get(),&argL) ;
    else if (arg ==         "cp") Console::        Cp::invoke(rpi.get(),&argL) ;
    else if (arg ==        "dma") Console::       Dma::invoke(rpi.get(),&argL) ;
    else if (arg ==       "gpio") Console::      Gpio::invoke(rpi.get(),&argL) ;
    else if (arg ==       "intr") Console::      Intr::invoke(rpi.get(),&argL) ;
    else if (arg ==    "max7219") Console::   Max7219::invoke(rpi.get(),&argL) ;
    else if (arg ==       "mbox") Console::      Mbox::invoke(rpi.get(),&argL) ;
    else if (arg ==    "mcp3008") Console::   Mcp3008::invoke(rpi.get(),&argL) ;
    else if (arg ==       "poke") Console::      Poke::invoke(rpi.get(),&argL) ;
    else if (arg ==        "pwm") Console::       Pwm::invoke(rpi.get(),&argL) ;
    else if (arg ==     "sample") Console::    Sample::invoke(rpi.get(),&argL) ;
    else if (arg ==        "shm") Console::       Shm::invoke(rpi.get(),&argL) ;
    else if (arg ==       "spi0") Console::      Spi0::invoke(rpi.get(),&argL) ;
    else if (arg ==       "spi1") Console::      Spi1::invoke(rpi.get(),&argL) ;
    else if (arg ==  "spi-slave") Console::  SpiSlave::invoke(rpi.get(),&argL) ;
    else if (arg == "throughput") Console::Throughput::invoke(rpi.get(),&argL) ;
    else if (arg ==    "ws2812b") Console::   Ws2812b::invoke(rpi.get(),&argL) ;
  
    else throw std::runtime_error("not supported option:<"+arg+'>') ;
  }
  catch (std::exception &error) {
    std::cerr << "exception caught:" << error.what() << std::endl ;
  }
  
  return 0 ;
  // [note] keep in mind that exceptions might also occur after returning
  //   from main() while cleaning up static variables
}
