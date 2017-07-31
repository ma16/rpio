# BSD 2-Clause License, see github.com/ma16/rpio

CXX=g++

INCLUDE=

CFLAGS=\
	-fno-strict-aliasing \
	-g \
	-I. \
	-O2 \
	-pedantic \
	-std=c++11 \
	-Wall \
	-Wconversion \
	-Wextra \
	-Wsign-conversion \
	-Wstrict-aliasing

# -DNDEBUG -flto
# [note] use -Wno-maybe-uninitialized when placement-new causes warnings

LDFLAGS=-pthread
# + $(CFLAGS) if -flto is used

CC=$(CXX) $(INCLUDE) $(CFLAGS)

LD=$(CXX) $(LDFLAGS)

LSRC=\
	Console/Clock/invoke.cc \
	Console/Cm/invoke.cc \
	Console/Device/invoke.cc \
	Console/Device/Ads1115/invoke.cc \
	Console/Device/Mcp3008/invoke.cc \
	Console/Device/Max7219/invoke.cc \
	Console/Device/Ws2812b/invoke.cc \
	Console/Dma/Lib.cc \
	Console/Dma/invoke.cc \
	Console/Gpio/invoke.cc \
	Console/Intr/invoke.cc \
	Console/Mbox/invoke.cc \
	Console/Poke/invoke.cc \
	Console/Pwm/invoke.cc \
	Console/Pwm/Lib.cc \
	Console/Sample/invoke.cc \
	Console/Shm/invoke.cc \
	Console/Spi0/Dma.cc \
	Console/Spi0/Dump.cc \
	Console/Spi0/invoke.cc \
	Console/Spi0/Layout.cc \
	Console/Spi0/Mosi.cc \
	Console/Spi0/Options.cc \
	Console/Spi0/Watch.cc \
	Console/Spi1/invoke.cc \
	Console/SpiSlave/invoke.cc \
	Console/Throughput/Buffer.cc \
	Console/Throughput/invoke.cc \
	Device/Ads1115/Bang/Generator.cc \
	Device/Ads1115/Bang/Host.cc \
	Device/Ads1115/Bang/Record.cc \
	Device/Max7219/Bang.cc \
	Device/Max7219/Parser.cc \
	Device/Mcp3008/Bang.cc \
	Device/Mcp3008/Spi0.cc \
	Device/Mcp3008/Spi1.cc \
	Device/Ws2812b/BitStream.cc \
	Linux/base.cc \
	Linux/PhysMem.cc \
	Linux/Shm.cc \
	Neat/Bit/Word.cc \
	Neat/NotSigned.cc \
	Neat/safe_int.cc \
	Neat/stream.cc \
	Posix/base.cc \
	Posix/Fd.cc \
	Posix/MMap.cc \
	Posix/shm.cc \
	Posix/Signal.cc \
	Rpi/ArmMem.cc \
	Rpi/Bus/Alloc.cc \
	Rpi/Cm.cc \
	Rpi/Counter.cc \
	Rpi/Dma/Channel.cc \
	Rpi/Function.cc \
	Rpi/Gpio.cc \
	Rpi/Mbox/Interface.cc \
	Rpi/Mbox/Message.cc \
	Rpi/Mbox/Memory.cc \
	Rpi/Mbox/Property/base.cc \
	Rpi/Mbox/Property/Clock.cc \
	Rpi/Mbox/Property/Device.cc \
	Rpi/Mbox/Property/Memory.cc \
	Rpi/Mbox/Property/Ram.cc \
	Rpi/Mbox/Queue.cc \
	Rpi/Page.cc \
	Rpi/Peripheral.cc \
	Rpi/Pwm.cc \
	Rpi/Spi0.cc \
	Rpi/Spi1.cc \
	Rpi/Timer.cc \
	RpiExt/Bang.cc \
	RpiExt/Pwm.cc \
	RpiExt/Serialize.cc \
	RpiExt/Spi0.cc \
	RpiExt/VcMem.cc \
	Ui/ArgL.cc \
	Ui/strto.cc

BSRC=\
	Console/rpio.cc

-include Makefile.default

