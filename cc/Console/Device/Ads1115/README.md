# ADS1115

The ADS1115 is an I2C driven 15-bit (+sign) ADC with 2x2 or 4x1 multiplexed channels and a maximum sampling rate of somewhat above 100/s. 

Please refer to the ADS1115 [datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf) for details.

## I2C (Brief Abstract)

I2C is an interface that supports multiple master and slave devices on a single bus. It has two wires: SCL and SDA. Both are pulled-up by resistor (open-drain).  To send a bit on the bus, SDA is driven by the sender to the appropriate level. Then a clock-pulse (rise+fall) is put by the master on SCL.

All data are transmitted in groups of eight bits (byte). When a sender has finished transmitting a byte, it stops driving SDA. The receiver acknowledges the byte by pulling SDA low for the next clock-pulse. A not-acknowledge is performed by leaving SDA high during the acknowledge cycle. 

Only a master can start a communication by initiating a START condition on the bus. Normally, SDA is only allowed to change its state while SCL is low. If SCL changes while the SCL is high, it is either a START or a STOP condition. 

* A START condition occurs when SCL is high and SDA goes from high to low. 
* A STOP condition occurs when SCL is high and SDA goes from low to high.

(The bus times out if the bus is held idle for more than 25ms.)

After the master issues a START condition, it sends an address byte. Each slave has a unique 7-bit address. Together with the address, a bit is sent that indicates whether the master wishes to read from (high) or write to (low) the slave.

If there is no slave with the requested address, there will be no acknowledge bit. Iterating thru all 128 addresses enables a master to browse for all connected slaves.

When the master has finished communication with a slave, it may issue a STOP condition and the bus becomes idle. The master may also issue another START condition while the bus is active, which is called a repeated START condition.

See http://www.i2c-bus.org for more details.

## Implementation

Tests were performed with an ADS1115 on a breakout board. SCL pulled with 10k to VDD, SDA pulled with 10k to VDD, ADDR pulled with 10k to GND.

The I2C address is configurable as:
* 0x48: ADDR to GND
* 0x49: ADDR to VDD
* 0x4a: ADDR to SDA
* 0x4b: ADDR to SCL

Hence, in the examples below, 0x48 is used as the I2C address.

Note (datasheet): The ADS1115 analog inputs are protected by diodes to the supply rails. However, the current-handling ability of these diodes is limited, and the circuit can be permanently damaged by analog input voltages that remain more than approximately 300mV beyond the rails for extended periods. One way to protect against overvoltage is to place current-limiting resistors on the input lines. The analog inputs can withstand momentary currents as large as 100mA.

The circuit supports three speed modes: standard 100 kHz, fast 400kHz, high-speed 3.4 MHz. Latter requires arbitration. This implementation bit-bangs with the fast timings of the datasheet. However, the implementation does not achive the minimum timings (measured ~350 kHz). The timing parameters are not configurable (yet).

Please note that bing-banged operation can be suspended at any time by the operating system.

This implementation does only support the "single-shot" mode. It does not support continuous mode. It does not support comparator configuration.

## ADS1115 Operation

You may want to refer to the datasheet. This is only a short excerpt.

There are three commands:
* Setup register pointer: START + WRITE + REGNO + STOP.
* Write the register that is pointed to: START + WRITE + MSB + LSB + STOP.
* Read the register that is pointed to: START + READ + MSB + LSB + STOP.

Every write must be  preceded by a write to the register pointer, even if the register number doesn't change. For a read sequence however, it is sufficient to set the register pointer just once.

Pointer Register (WO):
* 0 - conversion register 
* 1 - config register 
* 2 - lo-threshold register
* 3 - hi-threshold register

The conversion register (RO) holds the last taken sample as signed 16-bit integer.

Config Register (RW, default value 0x8583):
* 5-bit COMP
* 3-bit DR
* 1-bit MODE
* 3-bit PGA
* 3-bit MUX
* 1-bit OS

Data-rate in samples per second (DR):
* 0x0000: 8 
* 0x0020: 16
* 0x0040: 32
* 0x0060: 64
* 0x0080: 128 (default)
* 0x00a0: 250
* 0x00c0: 475
* 0x00e0: 860

Mode:
* 0x0000: Continuous conversion mode
* 0x0100: Power-down single-shot mode (default)

Programmable gain amplifier in +/- volts (PGA):
* 0x0000: 6.144
* 0x0200: 4.096
* 0x0400: 2.048 (default)
* 0x0600: 1.024
* 0x0800: 0.512
* 0x0a00: 0.256
* 0x0c00: 0.256
* 0x0e00: 0.256

Multplexer (MUX) to sample channels:
* 0x0000: 0,1 (default)
* 0x1000: 0,3
* 0x2000: 1,3
* 0x3000: 2,3
* 0x4000: 0,GND
* 0x5000: 1,GND
* 0x6000: 2,GND
* 0x7000: 3,GND

The single-ended circuit covers only half of the ADS1115 input scale because it does not produce differentially negative inputs; therefore, one bit of resolution is lost.

Operational status/single-shot conversion start (OS):
* Write:
* * 0x8000: Begin a single conversion (when in power-down mode)
* Read:
* * 0x0000: Device is currently performing a conversion
* * 0x8000: Device is not currently performing a conversion

## Synopsis
```
$ ./rpio device ads1115 help
arguments: SCL SDA ADDR [-m] MODE

 SCL: Pi's pin to set the ADS1115's SCL pin
 SDA: Pi's pin to r/w the ADS1115's SDA pin
ADDR: 0..127

MODE : config       # read configuration register
     | config WORD  # write configuration register
     | rate N       # determine sample-rate
     | reset        # write reset command
     | sample       # read sample register
```

The Raspberry needs to be setup beforehand.

## Setup (Example)
```
$ ./rpio clock set on 0
$ ./rpio gpio pull -l 22,23 off
$ ./rpio gpio output -l 22,23 lo
```
This does:
* Setup the ARM counter to the core-clock (normally 250 MHz)
* Pulls off the pins to use for the I2C communication (optional)
* Set the output level of the I2C pins to Low.

This implementation toggles the pin mode, not the output level. That is, in order to switch a pin level to High, the mode of the pin is changed to Input. To switch to Low, it is changed to Output.

## Examples

Read configuration:
```
$ ./rpio device ads1115 22 23 0x48 config 
success: 8583
```
This is the default configuration value (see above).

Single-shot sample:
```
$ ./rpio device ads1115 22 23 0x48 config 0x8583
success
$ ./rpio device ads1115 22 23 0x48 sample
success: 15939
```
This is a sample of a source with about 1 volt.

Determine sample rate:
```
$ ./rpio device ads1115 22 23 0x48 rate 100
1.22e+02/s (9.62e-01)
```
This uses the default configuration value of 0x8583 to initiate a single-shot sample and waits until the sampled value is available. This is done 100 times. The sample rate is about 120/s and the average measured value is about 1 volt (in parenthesis).
