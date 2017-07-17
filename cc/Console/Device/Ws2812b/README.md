# WS2812B

A cascadable controller for daisy chained RGB LEDs.

Please refer to the datasheet for details.

Note that a level-shifter is required to comply with the the datasheet. Still, it probably will work without.

The present implementation only supports to set-up a single GRB value (green-red-blue) for all LEDs of the chain (proof of concept).

## Synopsis

```
$ ./rpio device ws2812b help
arguments: MODE

MODE : bang NLEDS GRB [-t TIMING] [-f FREQ] PINS [-r RETRY] [-d]
     | pwm  NLEDS GRB [-t TIMING] [-f FREQ] CHANNEL [-d]
     | spi0 NLEDS GRB [-t TIMING] FREQ

TIMING : -t T1..T5

T1 = the 0-bit's High-level duration
T2 = the 0-bit's  Low-level duration
T3 = the 1-bit's High-level duration
T4 = the 1-bit's  Low-level duration
T5 = the latch (reset) duration
Min and max values have to be provided for T1..T4 in bit-banged mode

All values in seconds
Default values are the ones on the datasheet.

-d : display debug information
```

## Bit-Banging

The implementation barely keeps up with the required 400ns + 800ns bit-pulses. However, it might work with small chains of LED lights, at the costs of a high number of retries.

The bit-banged implementation relies on the ARM-counter. The counter needs to be set-up beforehand:
```
$ ./rpio clock set on 0
```
That enables the ARM-counter w/o divider @ core-clock with normally 250 MHz.

The pin to control the data line needs to be set-up too:
```
$ ./rpio gpio mode 22 o
```
In this example, pin number 22 is used to drive the data line.

Set 30 LEDs of a chain to the brightest white:
```
$ ./rpio device ws2812b bang 30 0xffffff 22 -r 10000
failed
```
The maximum number of retrys is given with 10,000. However, even this high number of iterations wasn't sufficient (in this call) to set-up all the LEDs. Another call may succeed.

On a multi-core processor, you may want to prevent execution on cpu no. 1 which appears to be seized for interrupt handling by the kernel. Cpu no. 3 appears to be a safe bet.

```
$ taskset -c 3 ./rpio device ws2812b bang 30 0x0 22 -r 10000
```

When the debug option is provided, the timings and the number of executed iterations are shown:
```
$ ./rpio device ws2812b bang 30 0xffffff 22 -r 10000 -d
f=2.50e+08
timing (seconds)=0-bit:((2.5e-07,5.5e-07),(7e-07,1e-06)) 1-bit:((6.5e-07,9.5e-07),(3e-07,6e-07)) latch:5e-05
timing (ticks)=0-bit:((63,137),(176,250)) 1-bit:((163,237),(76,150)) latch:12503
iterations: 68
```
In the case above, the set-up succeeded in the 68th iteration.

Even if the setup succeeds, the LEDs may not necessarily reflect the set-up values. You may see glitches:

The pulse for a 0-bit has a short High phase and a long Low phase (1:2). The pulse for a 1-bit is reversed (2:1). The datsheet defines maximum and minimum values for the levels of a pulse. This may lead to a duty-cycle that is almost balanced if the limits are reached: a duty-cycle of 5.5:7 instead of 1:2 and even 6.5:6 instead of 2:1. Not all circuits appear to tolerate this.

You may want to configure more restrictive timing limits:
```
$ ./rpio device ws2812b bang 30 0xffffff -t 20e-8 50e-8 70e-8 100e-8 70e-8 100e-8 20e-8 50e-8 5e-5 22 -r 10000 -d
f=2.50e+08
timing (seconds)=0-bit:((2e-07,5e-07),(7e-07,1e-06)) 1-bit:((7e-07,1e-06),(2e-07,5e-07)) latch:5e-05
timing (ticks)=0-bit:((51,125),(176,250)) 1-bit:((176,250),(51,125)) latch:12502
```
The duty cylce won't drop beyond 5:7 and 7:5. No glitches were observed with those values.

## PWM

This PWM implementation uses only one channel of the two channels and is cpu-controlled. Since it is not DMA controlled, it will fail on process suspension that leads to a PWM FIFO underrun.

In order to use PWM, you may need to disable the audio system in /boot/config.txtand reboot the system:
```
dtparam=audio=off
```

The PWM clock needs to be set-up beforehand:
```
$ ./rpio cm set 3 -i 200 -m 0 -s 6
$ ./rpio cm switch 3 on
```
In the example above, PLLD (@500.0 MHz) is employed with a divider of 200. So the effective PWM clock is 2.5 MHz. You can measure the clock by command:
```
$ ./rpio pwm freq
2.50e+06
```

We also have to enable the PWM pin (here pin 12 for channel no. 0) that drives the data line:
```
$ ./rpio gpio mode 12 0
```

Switch 30 LEDs on and off again for channel no. 0:
```
$ ./rpio device ws2812b pwm 30 0xffffff 0
$ ./rpio device ws2812b pwm 30 0x0 0
```

## SPI0

This SPI implementation is cpu-controlled. Since it is not DMA controlled, it will fail on process suspension that leads to a SPI FIFO underrun.

The SPI0 peripheral needs to be set-up beforehand:
```
$ ./rpio gpio mode 10 0
$ ./rpio spi0 control ren 0
$ ./rpio spi0 dlen 2
$ ./rpio spi0 div 100
```
That is:
* Enable GPIO pins 10 for SPI0 (MOSI) to drive the data line.
* Disable the read-enable-mode (which is active by default).
* Set the DLEN register to a value greater than 1. Otherwise there will be gaps between octets and mess up the signal.
* Set clock pulse to 250e+6/100 Hz.

Switch 30 LEDs on and off again. The active clock pulse needs to be provided by command line (here 2.5e+6 MHz):
```
$ ./rpio device ws2812b spi0 30 0xffffff 2.5e+6
$ ./rpio device ws2812b spi0 30 0x0 2.5e+6
```
