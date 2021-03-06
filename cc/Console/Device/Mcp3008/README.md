# MCP3008

The ADC supports medium sample-rates on 8x1 or 4x2 channels.

In order to properly use the command-line tool, you should be familiar with the MCP3008 datasheet.

## Supported Peripherals

```
$ ./rpio device mcp3008
arguments: ( bang | spi0 | spi1 ) [help]

bang: use bit-banging
spi0: use main SPI controller
spi1: use auxilliary SPI controller

SOURCE :  0  # differential CH0 = IN+ CH1 = IN-
       |  1  # differential CH0 = IN- CH1 = IN+
       |  2  # differential CH2 = IN+ CH3 = IN-
       |  3  # differential CH2 = IN- CH3 = IN+
       |  4  # differential CH4 = IN+ CH5 = IN-
       |  5  # differential CH4 = IN- CH5 = IN+
       |  6  # differential CH6 = IN+ CH7 = IN-
       |  7  # differential CH6 = IN- CH7 = IN+
       |  8  # single-ended CH0
       |  9  # single-ended CH1
       | 10  # single-ended CH2
       | 11  # single-ended CH3
       | 12  # single-ended CH4
       | 13  # single-ended CH5
       | 14  # single-ended CH6
       | 15  # single-ended CH7
```

## Bit-Banging
```
$ ./rpio device mcp3008 bang
arguments: CS DIN DOUT CLK [-m] [-f FREQ] MODE

  CS: Pi's pin to feed MCP's   CS pin @10
 DIN: Pi's pin to feed MCP's  DIN pin @11
DOUT: Pi's pin to read MCP's DOUT pin @12
 CLK: Pi's pin to feed MCP's  CLK pin @13

-m: enable monitoring to detect communication problems

FREQ: ARM counter frequency that has been set up

MODE : rate N [-s SOURCE]  # perform throughput test
     | sample SOURCE+      # read one or more samples

N - the number of consecutive samples to take
SOURCE - the MCP3008-channel to sample (0..15)
```

The applied timing parameters are the ones of the datasheet @ 3 volts. 

Currently no command line arguments are supported to change the default timing.

Note: Bit-banging relies on the ARM-counter. It needs to be set up properly beforehand. For example: Enable the ARM counter and set the maximum clock-speed (normally 250 MHz) by using the divider 0:
```
$ ./rpio clock set on 0
```

An error may be thrown if monitoring is enabled. The error codes are:
- Bit:0 - CS pin not High at reset (read-back)
- Bit:1 - CLK pin not Low at reset (read-back)
- Bit:2 - DIN pin not Low at reset (read-back)
- Bit:3 - DOUT pin not High after reset
- Bit:4 - DOUT pin start-bit not Low
- Bit:5 - MSB sample does not match LSB sample
- Bit:6 - DOUT pin not High after CS was disabled
- Bit:7 - Capacitance began to bleed out (timeout)

### Sample Data (Example)

Sample all sources:
```
$ ./rpio device mcp3008 bang 22 23 24 25 sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
info: measured frequency: 2.50e+08
1023 0 0 0 0 0 0 0 1023 0 0 0 0 0 0 0 
```
Pin 0 is set to 3.3 volts which is also the reference voltage and the MCP's power supply (taken from the Pi's pin header). All other pins (1..7) are connected to ground.

(The ARM counter runs at a frequency of 250 MHz.)

Sample all sources. Monitoring is enabled:
```
$ ./rpio device mcp3008 bang 22 23 24 25 -m sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
info: measured frequency: 2.50e+08
(1023,1023,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (1023,1023,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) 
```
This provides a 3-tuple for each source: The MSB sample, the LSB sample and an error code (0 on success). 

If monitoring is enabled then also the 9 LSB digits are transferred besides the 10 MSB digits. This increases the number of clock-pulses per transfer from 16 to 25, and thus, reduces the throughput. 

### Measure Throughput (Example)

This implementation of bit-banging provides about 80k samples per second (k/s):
```
$ ./rpio device mcp3008 bang 22 23 24 25 rate 100000
info: measured frequency: 2.50e+08
7.96e+04/s
```

It's about 50k/s when monitoring is enabled:
```
$ ./rpio device mcp3008 bang 22 23 24 25 -m rate 100000
info: measured frequency: 2.50e+08
success: 100000
5.08e+04/s
```
The program performs error checks to verify whether the transfer was successful. In the example above, all checks cleared (error code 0).

## Main Peripheral Controller (SPI0)
```
$ ./rpio device mcp3008 spi0
arguments: [-m] MODE

-m: enable monitoring to detect communication problems

MODE : rate N [-s SOURCE]  # perform throughput test
     | sample SOURCE+      # read one or more samples

N - the number of consecutive samples to take
SOURCE - the MCP3008-channel to sample (0..15)
```
Note: The SPI0 controller has to be set up beforehand (see example below).

SPI0 deals with a multiple of 8-bit octets. If monitoring is enabled then 4 octets are transferred. Otherwise only 3 octets. An error code is provided if something went wrong. The error codes are:
- Bit:0 - Set if the leading 7 (DOUT/MISO) digits are not 1111:110 (Binary).
- Bit:1 - Set if the (DOUT/MISO) MSB value does not match the LSB value.
- Bit:2 - Set if the trailing 6 (DOUT/MISO) digits are not zero.

This implementation does not (yet) use DMA for SPI0 invocation. SPI0 is used in direct mode only. That is, the transfer is controlled by the CPU, which includes busy loops for polling. Whenever the process is suspended by the operating system, this will affect the transfer (i.e. the timing). A single SPI dialog (SCLK,MOSI,MISO) is however safe to be not interrupted since the SPI serializer provides enough buffer space. The CS signals (CE0,CE1) can be delayed though.

### Controller Setup (Example)

Set up the SPI0 controller:
```
$ ./rpio gpio mode -l 7,8,9,10,11 0
$ ./rpio spi0 control ren 0
$ ./rpio spi0 dlen 2
$ ./rpio spi0 div 186
```
These four commands do:
* Enable GPIO pins 7..11 for SPI0, i.e. CE1, CE0, MISO, MOSI, SCLK.
* Disable the read-enable-mode (which is active by default).
* Set the DLEN register to a value greater than 1. Otherwise there will be gaps between octets; which wouldn't be a problem for the MCP3008 either, but slightly reduce the transfer rate.
* Set SCLK pulse to 2.5e+8/186, which is about the maximum clock speed of 1.35 MHz for the MCP3008 at 3 volts.

The default setup uses CE0 for the chip select signal. That can be changed:
```
$ ./rpio spi0 control cs 0 # use CE0
$ ./rpio spi0 control cs 1 # use CE1
```

### Sample Data (Example)

Sample all sources:
```
$ ./rpio device mcp3008 spi0 sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
1023 0 0 0 0 0 0 0 1023 0 0 0 0 0 0 0
```
Pin 0 is connected to the reference voltage. All other pins are connected to ground.

Sample all sources. Monitoring is enabled:
```
$ ./rpio device mcp3008 spi0 -m sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
(1023,fdffffc0,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (1023,fdffffc0,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) (0,fc000000,0) 
```
This provides a 3-tuple for each source: The sampled value, 32-bit DOUT/MISO data that were received (hex) and an error (hex) code (0 on success). 

### Measure Throughput (Example)

As mentioned before, the maximum clock speed for the MCP3008 at 3 volts is 1.35e MHz (according to the datasheet). Hence the SCLK pulse shouldn't be set any higher than 2.5e+8/186.
```
$ ./rpio device mcp3008 spi0 rate 100000
5.29e+04/s
$ ./rpio device mcp3008 spi0 -m rate 100000
success: 100000
4.03e+04/s
```
This permits sampling at about 40k/s with monitoring enabled and at about 53k/s w/o monitoring.

We might try to increase the clock speed beyond the specified limits until we get errors:
```
$ ./rpio spi0 div 48
$ ./rpio device mcp3008 spi0 -m rate 100000
success: 100000
1.45e+05/s
$ ./rpio spi0 div 47
$ ./rpio device mcp3008 spi0 -m rate 100000
error 0x2: 28
error 0x4: 18
success: 99954
1.50e+05/s
```
So, this model of the MCP3008 appears to be still working at 5.2 MHz (divider 48) with 3.3 volts power supply. That's however only half of the truth. Even though the transfer still works, the analog sampling doesn't keep up:
```
$ ./rpio spi0 div 48
$ ./rpio device mcp3008 spi0 sample 8
1018
```
Instead of the maximum count of 1023, the count is only 1018.

A clock speed of 4.5 MHz (divider 54) solves the problem:
```
$ ./rpio spi0 div 54
$ ./rpio device mcp3008 spi0 sample 8
1023 
```

## Auxilliary Peripheral Controller (SPI1)
```
$ ./rpio device mcp3008 spi1
arguments: [-m] MODE

-m: enable monitoring to detect communication problems

MODE : rate N [-s SOURCE]  # perform throughput test
     | sample SOURCE+      # read one or more samples

N - the number of consecutive samples to take
SOURCE - the MCP3008-channel to sample (0..15)
```

Note: The SPI1 controller has to be set up beforehand (see example below).

If monitoring is enabled then 26 bits are transferred. Otherwise only 17 bits. An error code is provided if something went wrong. The error codes are:
- Bit:0 - Set if the leading 7 (DOUT/MISO) digits are not 1111:110 (Binary).
- Bit:1 - Set if the (DOUT/MISO) MSB value does not match the LSB value.

SPI1 does not support DMA. So it is used in direct mode. That is, the transfer is controlled by the CPU, which includes busy loops for polling. Whenever the process is suspended by the operating system, this will affect the transfer (i.e. the timing). A single SPI dialog (SCLK,MOSI,MISO) is however safe to be not interrupted since the SPI serializer provides enough buffer space. The CS signals (CS0,CS1,CS2) can be delayed though.

### Controller Setup (Example)

The SPI1 controller needs to be enabled by Raspbian at boot time.
```
$ grep spi1 /boot/config.txt 
dtoverlay=spi1-1cs
```

Enabling the controller thru the Mailbox interface ([raspberrypi.org](https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=187187)) didn't work out yet.

Set up the SPI1 controller:
```
$ ./rpio gpio mode -l 16,17,18,19,20,21 4
$ ./rpio spi1 control tx-msb 1 rx-msb 1 speed 92
```

These commands do:
* Enable GPIO pins 16..21 for SPI1, i.e. CS2,CS1,CS0,MISO,MOSI and SCLK.
* Transceive MSB first; set SCLK pulse to 125e+6/(92+1), which is about the maximum clock speed of 1.35 MHz for the MCP3008 at 3 volts.

In the default setup, chip select signals CS0-2 are (concurrently) enabled. That can be changed:
```
$ ./rpio spi1 control cs#0 0  #  enable CS0
$ ./rpio spi1 control cs#0 1  # disable CS0
$ ./rpio spi1 control cs#1 0  #  enable CS1
$ ./rpio spi1 control cs#1 1  # disable CS1
$ ./rpio spi1 control cs#2 0  #  enable CS2
$ ./rpio spi1 control cs#2 1  # disable CS2
```

### Sample Data (Example)

Sample all sources:
```
$ ./rpio device mcp3008 spi1 sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
1023 0 0 0 0 0 0 0 1023 0 0 0 0 0 0 0
```
Pin 0 is connected to the reference voltage. All other pins are connected to ground.

Sample all sources. Monitoring is enabled:
```
$ ./rpio device mcp3008 spi1 -m sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
(1023,3f7ffff,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (1023,3f7ffff,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0) (0,3f00000,0)
```
This provides a 3-tuple for each source: The sampled value, 32-bit DOUT/MISO data that were received (hex) and an error (hex) code (0 on success). 

### Measure Throughput (Example)

As mentioned before, the maximum clock speed for the MCP3008 at 3 volts is 1.35e MHz (according to the datasheet). Hence the SCLK pulse shouldn't be set any higher than 2.5e+8/186.
```
$ ./rpio device mcp3008 spi0 rate 100000
6.59e+04/s
$ ./rpio device mcp3008 spi0 -m rate 100000
success: 100000
4.58e+04/s
```
This permits sampling at about 46k/s with monitoring enabled and at about 66k/s w/o monitoring.

We might try to increase the clock speed beyond the specified limits until we get errors:
```
$ ./rpio spi1 control speed 15
$ ./rpio device mcp3008 spi1 -m rate 100000
success: 100000
2.33e+05/s
$ ./rpio spi1 control speed 14
$ ./rpio device mcp3008 spi1 -m rate 100000
error 0x2: 2
success: 99998
2.43e+05/s
```
So, this model of the MCP3008 appears to be still working at 7.8 MHz (divider 15) with 3.3 volts power supply. That's however only half of the truth. Even though the transfer still works, the analog sampling doesn't keep up:
```
$ ./rpio spi1 control speed 15
$ ./rpio device mcp3008 spi1 -m sample 8
(839,3f68f8b,0) 

```
Instead of the maximum count of 1023, the count is only 839.

A clock speed of 3.6 MHz (divider 34) solves the problem:
```
$ ./rpio spi1 control speed 34
$ ./rpio device mcp3008 spi1 -m sample 8
(1023,3f7ffff,0) 
```
