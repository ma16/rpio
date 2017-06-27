# MCP3008

The ADC supports medium sample-rates on 8x1 or 4x2 channels.

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
$ sudo ./rpio device mcp3008 bang 
arguments: CS DIN DOUT CLK [-m] [-f FREQ] MODE

  CS: Pi's pin to feed MCP's   CS pin @10
 DIN: Pi's pin to feed MCP's  DIN pin @11
DOUT: Pi's pin to read MCP's DOUT pin @12
 CLK: Pi's pin to feed MCP's  CLK pin @13

-m: enable monitoring to detect communication problems

FREQ: ARM counter frequency

MODE : rate N [-s SOURCE]  # perform throughput test
     | sample SOURCE+      # read one or more samples

SOURCE - the MCP3008-channel to sample (0..15)
```

The applied timing parameters are the ones of the datasheet @ 3 volts. 

Currently no command line arguments are supported to change the default timing.

Note: Bit-banging relies on the ARM-counter. The clock needs to be set-up properly beforehand. 

For example, in order to set the maximum clock-speed (normally 250 MHz):
```
$ ./rpio clock set on 0
```

### Sample (Example)

Sample all sources:
```
$ ./rpio device mcp3008 bang 22 23 24 25 sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
info: measured frequency: 2.50e+08
1023 0 0 0 0 0 0 0 1023 0 0 0 0 0 0 0 
```
Pin 0 is connected to the reference voltage. All other pins are connected to ground.

(The ARM counter runs at a frequency of 2.5e+8 MHz.)

Sample all sources. Monitoring is enabled:
```
$ ./rpio device mcp3008 bang 22 23 24 25 -m sample 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
info: measured frequency: 2.50e+08
(1009,1009,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (1023,1023,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) (0,0,0) 
```
This provides a 3-tuple for each source: The MSB sample, the LSB sample and an error code (0 on success). The error codes are:

- Bit:0 - CS pin not High at reset (read-back)
- Bit:1 - CLK pin not Low at reset (read-back)
- Bit:2 - DIN pin not Low at reset (read-back)
- Bit:3 - DOUT pin not High after reset
- Bit:4 - DOUT pin start-bit not Low
- Bit:5 - MSB sample does not match LSB sample
- Bit:6 - DOUT pin not High after CS was disabled
- Bit:7 - Capacitance began to bleed out (timeout)

If monitoring is enabled then also the 9 LSB digits are transferred besides the 10 MSB digits. This increases the number of clock-pulses per transfer from 16 to 25, and thus, reduces the throughput. 

### Throughput (Example)

The ADC samples about 80.000 analog inputs per second:
```
$ ./rpio device mcp3008 bang 22 23 24 25 rate 100000
info: measured frequency: 2.50e+08
7.96e+04/s
```

It's about 50.000/s when monitoring is enabled:
```
$ ./rpio device mcp3008 bang 22 23 24 25 -m rate 100000
info: measured frequency: 2.50e+08
success: 100000
5.08e+04/s
```
The program performs some checks to verify whether the transfer was successful. In the example above, all checks cleared.
