# Clock Manager (CM)

The [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf) document does not include the Clock Manager. It describes only the three General Purpose GPIO Clocks (§6.3). However, there are more clock generators, for example for PWM and PCM. Please refer also to the errata section at the end of this page.

## Register Block

The base-address of the register-block is 7E10:1000.

The eLinux project lists 114 registers. These include:

| | CTL | DIV
:--- | ---: | ---:
GP0 | 70 | 74
GP1 | 78 | 7C
GP2 | 80 | 84
PCM | 98 | 9C
PWM | A0 | A4
SLIM | A8 | AC
UART | F0 | F4

Each of these clock generators is set-up by a control register and by a divider. 

The maximum operating frequency of the General Purpose clocks (GP0,GP1,GP2) is ~125MHz at 1.2V. This will be reduced if the GPIO pins are heavily loaded or have a capacitive load.

Don't do any changes to GP1 on Raspberry Pi models that provide an Ethernet interface. GP1 seems to provide the Ethernet clock.

## Control Register (CTL)

Offset | Bits | Access | Name | Description
---: | ---: | :--- | :--- | :---
0 | 4 | RW | SRC | Clock Source
4 | 1 | RW | ENAB | Enable and disable clock generator
5 | 1 | WO | KILL | Kill the clock generator
7 | 1 | RO | BUSY | Reflects whether the clock is running (or not)
8 | 1 | RW | FLIP | Invert output
9 | 2 | RW | MASH | Audio-Band Noise-Shaping
24 | 8 |WO | PASSWD | Writing the register succeeds only if set to 5A

### Clock Source (CTL.SRC)

To avoid lock-ups and glitches do not change this control while BUSY=1 and do not change this control at the same time as asserting ENAB. 

There are 16 sources:

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

Only source *1* and *6* appear to have the same frequency on all Raspberry Pi models. The frequency of PLLC and HDMI seems to vary.

### Start and Stop (CTL.ENAB)

This requests the clock to start or to stop. In order to avoid glitches, the clock will not stop immediately but finish the current cycle. The BUSY flag is cleared when the final cycle (of the current Low/High pulse) is completed. 

### Stop Immediately (CTL.KILL)

KILL=0 has no effect.

KILL=1: Stop and reset the clock generator. This is intended for test/debug only. Using this control may cause a glitch on the clock generator output.

Note, this is no single-shot operation. The application has to set the flag, and then to clear it again.

Sometimes the clock generator may get stuck. This was noticed on a Pi-2 with (SRC=1,MASH=0) when changing the divider. The clock pulse dropped to minimum rate (as if a divider of 4096 was used) and no new setting did change the rate. The only solution was to kill the clock generator. So use this flag to revive the clock generator.

### Status (CTL.BUSY)

The flag indicates whether the clock generator is running. To avoid glitches and lockups, configuration must not change while this flag is set. 

### Invert Output (CTL.FLIP)

The datasheet says: "This is intended for use in test/debug only. Switching this flag will generate an edge on the clock generator output. To avoid output glitches do not switch this control while BUSY=1."

So it says: asserting the flag generates an edge on the output signal but this must not be done when busy. Which makes not much sense. 

The flag is neither static to inverse output polarity. If the bit is set, it doesn't stick. It reads always back as zero.

So it seems that the flag has no effect at all. The eLinux project names the bit *BUSYD* instead of *FLIP*.

### Audio-Band Noise-Shaping (CTL.MASH)

The value controls whether the fractional part of the divider shall be used (DIV.FRACT), and if so, whether audio-band noise-shaping shall be applied.

If the fractional part of the divider is used, there will be a jitter in the output signal. The MASH noise-shaping can be applied to push the jitter out of the audio band by spreading the actual frequency around the requested frequency.

| # | Description
---: | :---
0 | The fractional part of the divider is not used at all.
1 | The fractional part of the divider is used, but no noise-shaping.
2 | The fractional part of the divider is used, and noise-shaping.
3 | The fractional part of the divider is used, and more noise-shaping.

Notes: 
* According to the datasheet, the noise shaping shall not be exposed to frequencies higher than 25 MHz.
* According to [eLinux](http://elinux.org/BCM2835_registers#CM), GP2 and UART do not support noise shaping; i.e. only values #0 and #1 are supported.

The frequency divider (DIV) fluctuates for MASH values above zero as follows:

| # | min | average | max
---: | :--- | :--- | :---
0 | INTGR   | INTGR                | INTGR
1 | INTGR   | INTGR + FRACT / 4096 | INTGR+1
2 | INTGR-1 | INTGR + FRACT / 4096 | INTGR+2
3 | INTGR-3 | INTGR + FRACT / 4096 | INTGR+4

Each of the MASH values imposes a minimum FRACT.INTGR value:

| # | INTGR
---: | :---
0 | >= 1
1 | >= 2
2 | >= 3
3 | >= 5

To avoid lock-ups and glitches do not change the MASH value while BUSY=1 and not at the same time as asserting CTL.ENAB. 

## Divider Register (DIV)

Offset | Bits | Access | Name | Alias | Description
---: | ---: | :--- | :--- | :--- | :---
0 | 12 | RW | DIVF | FRACT | Fractional part of divisor
12 | 12 | RW | DIVI | INTGR | Integer part of divisor
24 | 8 | WO | PASSWD | | Writing the register succeeds only if set to 5A

The INTGR divider [should](http://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks) be in the range 2..4095. The value 0 appears to disable the output signal. The value 1 appears to behave as a divider of 4096.

The fractional divider (FRACT) operates by periodically dropping clock pulses. Therefore the output frequency will switch between SRC/INTGR and SRC/(INTGR+1). The resulting jitter may affect the audio band (see CTL.MASH).

According to [eLinux](http://elinux.org/BCM2835_registers#CM), UART supports only 22 instead of 24 INTGR+FRACT bits. Bit:0-7 appear to be FRACT and Bit:8-21 appear to be INTGR.

## Use Cases

### Use-Case-1

In order to determine an oscillator frequency, the oscillator source is tapped to the PWM clock-generator; the PWM peripheral is switched into FIFO mode; and the time to clock-out a bit-stream is taken. The values above were measured on Pi-0/2/3 with different configurations. Treat them with caution. Anyway, the results correlate with those of other users at [stackexchange.com](https://raspberrypi.stackexchange.com/questions/1153/what-are-the-different-clock-sources-for-the-general-purpose-clocks#23277).

```
$ rpio cm ctl pwm -enab ;\
  rpio cm div pwm intgr 10 ;\
  rpio cm ctl pwm mash 0 src 1 +enab ;\
  rpio pwm range 1 1000 ;\
  rpio pwm control -pwen2 +usef1 +mode1 +pwen1 ;\
  rpio pwm frequency
1.92e+06 (1.92e+03,0)
```

This reports for clock (src=1) a rate of 1.92e+3 words per second. That makes 19.2 MHz with (range=1,000) bits per word and an (intgr=10) divider. 

```
$ rpio cm ctl pwm -enab ;\
  rpio cm div pwm intgr 10 ;\
  rpio cm ctl pwm mash 0 src 6 +enab ;\
  rpio pwm range 1 1000 ;\
  rpio pwm control -pwen2 +usef1 +mode1 +pwen1 ;\
  rpio pwm frequency
5.00e+07 (5.00e+04,0)
```
For clock (src=6) the rate is 500 MHz.

## Errata

This is the errata for the [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf) document. You may also want to have a look at the errata on [eLinux](http://elinux.org/BCM2835_datasheet_errata).

Page | Description
--- | ---
105 | The peripheral datasheet describes only three clock generators.
| | Many more have been disclosed by [eLinux](http://elinux.org/BCM2835_registers#CM). Some of them are vital since they drive other peripherals.
105 | DIVF / 1024
| | DIVF / 4096
107 | FLIP
| | The flag reads always as zero and has no apparent effect.
107 | SRC
| | The frequency of the clock-sources is not defined. See Use-Case-1 above.

You may also want to have a look at the Linux kernel [sources](https://github.com/raspberrypi/linux/blob/rpi-4.9.y/drivers/clk/bcm/clk-bcm2835.c).