# Peripheral Clock Manager

The [peripheral datasheet](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf) describes in chapter 6.3 "General Purpose GPIO Clocks" three clock-generators. However, many more have been disclosed by [eLinux](http://elinux.org/BCM2835_registers#CM). Some of them are vital since they drive other peripherals, for example PWM.

## Summary

Each clock generator is set-up by a control register and by a divider. 

### Control Register

Offset | Bits | Access | Name | Description
---: | ---: | :--- | :--- | :---
0 | 4 | RW | SRC | Clock source
4 | 1 | RW | ENAB | Enable and disable clock generator
5 | 1 |  W | KILL | Kill the clock generator
7 | 1 | R | BUSY | Reflects whether the clock is running (or not)
8 | 1 | RW | FLIP | Invert output
9 | 2 | RW | MASH | Filter to put fractional divider jitter out of the audio band
24 | 8 | W | PASSWD | Sequence to enable write-access

#### Clock Source

Basically, 16 clock-sources are available. However, the frequency of a clock-source is not defined in the datasheet.

In order to determine a frequency, the source is applied for the PWM clock-generator; the PWM peripheral is switched into FIFO mode; and the time to clock-out a bit-stream is taken. The values below were measured on Pi-0/2/3 with different configurations. Treat them with caution. 

Number | Name | Frequency
---: | :--- | :---
1 | oscillator | 1.92e+7
2 | testdebug0 | 0.00e+0
3 | testdebug1 | 0.00e+0
4 | PLLA | 0.00e+0
5 | PLLC | 1.00e+9 .. 1.20e+9 
6 | PLLD | 5.00e+8
7 | HDMI | 0.00e+0 .. (1.08e+08) .. 2.16e+8
other | GND | 0.00e+0

Only source *1* and *6* appear to have the same frequency on several Raspberry Pi models. The results correlate with those of [other people](https://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks#23277).

### Divider

Offset | Bits | Access | Name | Description
---: | ---: | :--- | :--- | :---
0 | 12 | RW | DIVF | Fractional part of divisor
12 | 12 | RW | DIVI | Integer part of divisor
24 | 8 | W | PASSWD | Sequence to enable write-access

## Synopsis

```
# ./rpio cm help
arguments: MODE [help]

MODE : set     # set up clock parameters
     | status  # display the status of all clock generators
     | switch  # switch clock generator on/off

There are four clock generators specified by ALIAS:
ALIAS: gp0 | gp1 | gp2 | pwm
```

### Set

```
# ./rpio cm set help
arguments: ALIAS [-f FRACT] [-i INT] [-m MASH] [-s SOURCE]

 FRACT : fractional part of the pre-scaler (0..FFF)
   INT :    integer part of the pre-scaler (0..FFF)
  MASH : apply 'mash' filter (0..3, where 0 means off)

SOURCE : the clock source (0..15); for example:
           1 = ocillator =    19.2 Mhz
           6 = PLLD      =   500.0 MHz
```

### Status

There are no command line arguments.

Example:

```
# ./rpio cm status
alias on bsy src div:i div:f mash
---------------------------------
gp0    0   0   0     0     0    1
gp1    1   1   6    20     0    0
gp2    0   0   0     0     0    0
pwm    1   0  15  1000     0    1
```

### Switch

```
# ./rpio cm switch help
arguments: ALIAS MODE

MODE: off | on | toggle | kill
```
