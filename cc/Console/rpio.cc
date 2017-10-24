// BSD 2-Clause License, see github.com/ma16/rpio

#include "rpio.h"
#include <Posix/base.h>
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

static void help()
{
    std::cout
	<< "arguments: [BASE] MODE [help]\n"
	<< '\n'
	<< "BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000\n"
	<< "     | --devtree       # use info in /proc/device-tree/soc/ranges\n"
	<< '\n'
	<< "If BASE is not given then the peripheral address is derived from\n"
	<< "the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.\n"
	<< '\n'
	<< "MODE | clock       # r/w clock sources\n"
	<< "     | cm          # clock-manager\n"
	<< "     | defect      # defect report\n"
	<< "     | device      # control a certain device\n"
	<< "     | dma         # DMA setup and tests\n"
	<< "     | gpio        # r/w GPIO\n"
	<< "     | intr        # enable/disable ARM and GPU interrupts\n"
	<< "     | mbox        # Mailbox access\n"
	<< "     | poke        # r/w any word in peripheral address space\n"
	<< "     | pwm         # PWM setup\n"
	<< "     | sample      # sample data\n"
	<< "     | shm         # shared memory control (POSIX IPC)\n"
	<< "     | spi0        # serial peripheral interface (controller #0)\n"
	<< "     | spi1        # serial peripheral interface (controller #1)\n"
	<< "     | spi-slave   # serial peripheral interface (slave)\n"
	<< "     | throughput  # i/o and memory performance tests\n"
	<< '\n'
	<< "Use the keyword help for additional information.\n"
	;
}

int main(int argc,char **argv)
{
    try
    {
	auto argL = Ui::ArgL::make(argc-1,argv+1) ;

	if (argL.empty() || argL.peek() == "help")
	{
	    help() ; return 0 ;
	}
    
	auto rpi = Rpi::Peripheral::make(base_addr(&argL)) ;
	Posix::reset_uid() ;

	using namespace Console ;
	std::map<std::string,void(*)(Rpi::Peripheral*,Ui::ArgL*)> map =
	{
	    { "clock",Clock::invoke },
	    { "cm",Cm::invoke },
	    { "defect",Defect::invoke },
	    { "device",Device::invoke },
	    { "dma",Dma::invoke },
	    { "gpio",Gpio::invoke },
	    { "intr",Intr::invoke },
	    { "mbox",Mbox::invoke },
	    { "poke",Poke::invoke },
	    { "pwm",Pwm::invoke },
	    { "sample",Sample::invoke },
	    { "shm",Shm::invoke },
	    { "spi0",Spi0::invoke },
	    { "spi1",Spi1::invoke },
	    { "spi-slave",SpiSlave::invoke },
	    { "throughput",Throughput::invoke },
	} ;
	argL.pop(map)(rpi.get(),&argL) ;
    }
    catch (std::exception &error)
    {
	std::cerr << "exception caught:" << error.what() << std::endl ;
    }

    return 0 ;
    // [note] keep in mind that exceptions might also occur after returning
    //   from main() while cleaning up static variables
}
