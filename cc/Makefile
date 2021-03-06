# BSD 2-Clause License, see github.com/ma16/rpio

CXX=g++

INCLUDE=

CFLAGS=\
	-DNDEBUG \
	-fno-strict-aliasing \
	-g \
	-I. \
	-O2 \
	-pedantic \
	-std=c++11 \
	-Wall \
	-Wconversion \
	-Wextra \
	-Wno-maybe-uninitialized \
	-Wsign-conversion \
	-Wstrict-aliasing

# -flto: doesn't work on ARM
# -Wno-maybe-uninitialized: boost::optional throws
# -Wzero-as-null-pointer-constant: boost::none throws

LDFLAGS=-lrt -pthread

CC=$(CXX) $(INCLUDE) $(CFLAGS)

LD=$(CXX) $(LDFLAGS)

LSRC=\
	Console/Defect/invoke.cc \
	Console/Defect/D1/invoke.cc \
	Console/Device/invoke.cc \
	Console/Device/Ads1115/invoke.cc \
	Console/Device/Ds18b20/invoke.cc \
	Console/Device/Mcp3008/invoke.cc \
	Console/Device/Max7219/invoke.cc \
	Console/Device/Ws2812b/invoke.cc \
	Console/Peripheral/invoke.cc \
	Console/Peripheral/ArmTimer/invoke.cc \
	Console/Peripheral/Cm/invoke.cc \
	Console/Peripheral/Dma/invoke.cc \
	Console/Peripheral/Gpio/invoke.cc \
	Console/Peripheral/Intr/invoke.cc \
	Console/Peripheral/Mbox/invoke.cc \
	Console/Peripheral/Pwm/invoke.cc \
	Console/Peripheral/Spi0/Dma.cc \
	Console/Peripheral/Spi0/Dump.cc \
	Console/Peripheral/Spi0/invoke.cc \
	Console/Peripheral/Spi0/Layout.cc \
	Console/Peripheral/Spi0/Mosi.cc \
	Console/Peripheral/Spi0/Options.cc \
	Console/Peripheral/Spi0/Watch.cc \
	Console/Peripheral/Spi1/invoke.cc \
	Console/Peripheral/SpiSlave/invoke.cc \
	Console/Poke/invoke.cc \
	Console/Sample/invoke.cc \
	Console/Sample/event.cc \
	Console/Sample/level.cc \
	Console/Shm/invoke.cc \
	Console/Throughput/Buffer.cc \
	Console/Throughput/invoke.cc \
	Device/Ads1115/Bang/Generator.cc \
	Device/Ads1115/Bang/Host.cc \
	Device/Ads1115/Bang/Record.cc \
	Device/Ds18b20/Bang.cc \
	Device/Max7219/Bang.cc \
	Device/Max7219/Parser.cc \
	Device/Mcp3008/Bang.cc \
	Device/Mcp3008/Spi0.cc \
	Device/Mcp3008/Spi1.cc \
	Device/Ws2812b/BitStream.cc \
	Linux/base.cc \
	Linux/PhysMem.cc \
	Linux/Shm.cc \
	Neat/Bit/Crc.cc \
	Neat/Bit/Digit.cc \
	Neat/Bit/Sequence.cc \
	Neat/Bit/Word.cc \
	Neat/Bit/WordOld.cc \
	Neat/Enum.cc \
	Neat/NotSigned.cc \
	Neat/safe_int.cc \
	Neat/stream.cc \
	Posix/base.cc \
	Posix/Fd.cc \
	Posix/MMap.cc \
	Posix/shm.cc \
	Posix/Signal.cc \
	Protocol/OneWire/Bang/Addressing.cc \
	Protocol/OneWire/Bang/Error.cc \
	Protocol/OneWire/Bang/Master.cc \
	Protocol/OneWire/Bang/Signaling.cc \
	Protocol/OneWire/Bang/Timing.cc \
	Rpi/ArmMem.cc \
	Rpi/Bus/Alloc.cc \
	Rpi/Cm.cc \
	Rpi/ArmTimer.cc \
	Rpi/Dma/Channel.cc \
	Rpi/Dma/Ti.cc \
	Rpi/Gpio/Function.cc \
	Rpi/Gpio/Pull.cc \
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
	Rpi/Register.cc \
	Rpi/RegOld.cc \
	Rpi/Spi0.cc \
	Rpi/Spi1.cc \
	Rpi/Timer.cc \
	RpiExt/Bang.cc \
	RpiExt/BangIo.cc \
	RpiExt/Dma/Control.cc \
	RpiExt/Pwm.cc \
	RpiExt/Serialize.cc \
	RpiExt/Spi0.cc \
	RpiExt/VcMem.cc \
	Rpi/Ui/Bus/Coherency.cc \
	Rpi/Ui/Bus/Memory.cc \
	Rpi/Ui/Dma.cc \
	Ui/ArgL.cc \
	Ui/strto.cc

BSRC=\
	Console/rpio.cc

-include Makefile.default

