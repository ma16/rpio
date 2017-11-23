# Digital Signal Sampling

Digital signal sampling in this context is the process of tracking a binary signal (high and low) over time. This may be useful to determine the (average) frequency or duty cylce, for monitoring purposes, or for communication (receiving data).

Often, data is sampled at a constant rate; for instance at a rate (R) of one million (samples) per second (1 M/s). This will detect all signal levels that last longer than 1/R = 1µs. Thus, a 1:1 square-wave-signal at 400 kHz (where each High and Low level lasts for 1.25 µs) will be tracked; a 1:11 square-wave-signal at 100 kHz however not.

Sampling at a constant rate is not possible thru a vanilla Linux userland process: A busy loop that polls an input register won't return values at a constant rate (unlike a loop on a microcontroller). The executing thread may be interrupted and suspended at any time for any duration by the operating system. Besides, depending on the bus and memory architecture, there may be unpredictable latencies (e.g. due to cache operations).

Anyway, this stub provides a few simple methods to sample data (at no contant rate):
* Level: the signal level is polled in a busy loop.
* Event: the event detect status register is polled in a busy loop.
* Inspect: similar to Event. However, retries on potential problems.
* Pulse: PWM measurement.

## Synopsis

```
$ ./rpio sample [help]
```

## Clocks

The Raspberry Pi uses several clocks (See [eLinux](https://elinux.org/RPiconfig)) to drive its bus systems.

Name  | Option     | Bus System
:---- | :--------- | :---------
ARM   | arm_freq   | ARM Core(s) that run Linux
CORE  | core_freq  | Peripheral Bus (APB)*
SDRAM | sdram_freq | SDRAM

(*) This clock is also used on the VideoCore and its L2-cache. All the recent Raspbian images use a Linux kernel that enables the VC's L2-cache for the ARM Core so the cache is shared between the ARM Core and the VideoCore; however, old kernels do not. This is only relevant for the Pi-0 and the Pi-1 variants. ARM cores on Pi-2 and Pi-3 have their own L2-cache.

Model   | ARM      | CORE    | SDRAM
:------ | :------- | :------ | :------
Default |      700 |     250 |     400
Pi-0    | 700-1000 | 250-400 | 400-450
Pi-2    | 600- 900 |     250 | 400-450
Pi-3    | 600-1200 | 250-400 | 400-450

(Default values by eLinux)

Sampling at fixed intervals isn't easy on the Raspberry Pi in the first place due to its architecture and due to a vanilla's Linux kernel's impacts on a userland process. Varying clocks (as shown above) make it even harder (if not to say impossible). Luckily, the (Raspbian) configuration in /boot/config.txt permits to make these clocks to pulse on a fixed frequency.

Fix clocks to maximum frequency:
```
force_turbo=1
```

Fix clocks to minimum frequency:
```
arm_freq=600
core_freq=250
sdram_freq=400
```
On a Pi-0, arm_freq can safely be set to 700 (MHz).

You can verify the clock frequencies yourself:
```
$ rpio peripheral mbox clock status
```

For example, on a Pi-3:

Default:
```
 # parent state         hz        min        max name
-----------------------------------------------------
 3      0     1  600000000  600000000 1200000000 ARM
 4      0     1  250000000  250000000  400000000 CORE
 8      0     1  400000000  400000000  450000000 SDRAM
```

If configuration sets maximum:
```
 # parent state         hz        min        max name
-----------------------------------------------------
 3      0     1 1200000000 1200000000 1200000000 ARM
 4      0     1  400000000  400000000  400000000 CORE
 8      0     1  450000000  450000000  450000000 SDRAM
```

If configuration sets minimum:
```
 # parent state         hz        min        max name
-----------------------------------------------------
 3      0     1  600000000  600000000  600000000 ARM
 4      0     1  250000000  250000000  250000000 CORE
 8      0     1  400000000  400000000  400000000 SDRAM
```

## Sample Input Level

The input level register (GPLEV0) is polled in a busy loop. The program expects the number of iterations and the GPIO pin to watch. There are two counters: One counter increments with each detected High level input. The other counter increments with each Level change (from High to Low and Low to High).

The program outputs:
* The sample rate (r) which is the number of iterations per second.
* The signal frequency (f) which is is the number of transitions per second divided by two.
* The duty cycle which is the High-level count divided by the number of iterations.

All values are rounded to three significant digits.

### Example

Configure GPIO pin 18 as alternate function 5 to connect PWM channel #1:
```
$ rpio peripheral gpio mode 18 5
```

Configure 10 MHz for the PWM base clock:
```
$ rpio peripheral cm ctl pwm -enab +kill &&\
  rpio peripheral cm div pwm intgr 50 fract 0 &&\
  rpio peripheral cm ctl pwm mash 0 src 6 -kill &&\
  rpio peripheral cm ctl pwm +enab
```

Configure a 1/10 pulse for PWM channel #1:
```
$ rpio peripheral pwm control -pwen1 &&\
  rpio peripheral pwm range 1 10 &&\
  rpio peripheral pwm data 1 1 &&\
  rpio peripheral pwm control -mode1 +msen1 -pola1 -usef1 +pwen1
```

Sample GPIO pin 18 (Pi-3):
```
$ rpio sample level 18 100000000
r=1.65e+07/s f=9.98e+05/s duty=1.00e-01
```
The sample rate was 16.5 M/s, the determined frequency 0.998 MHz (instead of 1 MHz) and the determined duty cycle 0.1.

Configure a 7/10 pulse for PWM channel #1:
```
$ rpio peripheral pwm data 1 7
```

Sample GPIO pin 18 (Pi-3):
```
$ rpio sample level 18 100000000
r=1.65e+07/s f=9.99e+05/s duty=7.01e-01
```
The sample rate was 16.5 M/s, the determined frequency 0.999 MHz (instead of 1 MHz) and the determined duty cycle 0.701 (instead of 0.7).

## Watch Events

The event detect status register (GPEDS0) is polled in a busy loop. The program arguments are the number of iterations and the GPIO pins to watch. All event registers (e.g. GPAREN0) must have been set up by the user beforehand. There are two counters: One counter increments with each detected event. (If an event is detected, it will be reset immediately by the program). If another event is detected in the subsequent iteration, the seond counter is incremented. The incrementation of the second timer stop if no event was detected.

The program outputs:
* The sample rate (r) which is the number of iterations per second.
* The signal frequency (f) which is the number of detected events per second.
* The sequential rate (s) which is the number of subsequent events per second.

All values are rounded to three significant digits.

The sample rate reflects the average time between samples. However, the actual time between two individual samples may vary tremendously. Thus, the sampled data is of limited use.

If you watch the rising edge and the falling edge of a square wave, you'll receive alternating events (fall, rise, fall, a.s.o.). If you sample these events, and you get delayed, you will receive both events (fall and rise) within a single sample. This method provides the means to detect delays in your application (though not prevent them) and the possibility to deal with them.

### Example

Configure GPIO pin 15 as input and pin 18 as alternate function 5 to connect PWM channel #1:
```
$ rpio peripheral gpio mode 15 i &&\
  rpio peripheral gpio mode 18 5
```

Connect GPIO pin 18 with GPIO pin 15 by a wire. We need a second pin since the peripheral permits only watching one event per pin (e.g. fall *or* rise).

Configure GPIO event detection for pin 15 and 18:
```
$ rpio peripheral gpio enable 15 fall &&\
  rpio peripheral gpio enable 18 rise 
```

Configure 10 MHz for the PWM base clock:
```
$ rpio peripheral cm ctl pwm -enab +kill &&\
  rpio peripheral cm div pwm intgr 50 fract 0 &&\
  rpio peripheral cm ctl pwm mash 0 src 6 -kill &&\
  rpio peripheral cm ctl pwm +enab
```

Configure a 50/100 pulse (1:1 square wave at 100 kHz) for PWM channel #1:
```
$ rpio peripheral pwm control -pwen1 &&\
  rpio peripheral pwm range 1 100 &&\
  rpio peripheral pwm data 1 50 &&\
  rpio peripheral pwm control -mode1 +msen1 -pola1 -usef1 +pwen1
```

Sample events (Pi-3):
```
$ rpio sample event -l 15,18 100000000
r=1.63e+07/s f=2.00e+05/s s=2.29e+00/s
```
The sample rate is 16.3 M/s. There are 200k events per second (100 kHz signal frequency). There were 2.29 subsequent events per seconds (which suggest a potential sampling delay).

Configure a 1/100 pulse (at 100 kHz) for PWM channel #1:
```
$ rpio peripheral pwm data 1 1
```

Sample events (Pi-3):
```
$ rpio sample event -l 15,18 100000000
r=1.64e+07/s f=1.96e+05/s s=8.37e+04/s
```
The sample rate is 16.4 M/s. There are 196k events per second (98 kHz signal frequency). There were more than 80k subsequent events per seconds which confirm the aberrated measurement due to a sampling delay.

## Sample a Single Pulse

The program expects the GPIO pin to watch as argument. It sets (and restores) the registers GPAREN0, GPAFEN0, and then again GPAREN0. The event detect status register (GPEDS0) is polled three times. First, for a rising edge, then for a falling edge, and then again, for a rising edge.

The program outputs:
* The interval (t0,+t1) of the rising edge.
* The interval (t2,+t3) of the subsequent falling edge.
* The interval (t4,+t5) of the subsequent rising edge.

Each interval depicts a time-point before and after the edge. (Intervals are used because accurate time-points aren't available.) The values t1, t2, and t4 are offsets to t0; t3 is an offset to t2; t5 is offset to t4. All values are rounded to three significant digits.

Note: It would also be possible to poll the input level register (instead of the event detect status register). However, with the help of the GPEDS you can make sure that no edge will be missed (once the setup is finished).

### Example

Configure GPIO pin 18 as alternate function 5 to connect PWM channel #1:
```
$ rpio peripheral gpio mode 18 5
```

Configure 10 MHz for the PWM base clock:
```
$ rpio peripheral cm ctl pwm -enab +kill &&\
  rpio peripheral cm div pwm intgr 50 fract 0 &&\
  rpio peripheral cm ctl pwm mash 0 src 6 -kill &&\
  rpio peripheral cm ctl pwm +enab
```

Configure a 40/1000 pulse for PWM channel #1:
```
$ rpio peripheral pwm control -pwen1 &&\
  rpio peripheral pwm range 1 1000 &&\
  rpio peripheral pwm data 1 40 &&\
  rpio peripheral pwm control -mode1 +msen1 -pola1 -usef1 +pwen1
```

Sample GPIO pin 18 (Pi-3):
```
$ rpio sample pulse 18 
0+4.17e-07 4.12e-06+4.16e-07 9.99e-05+4.17e-07
```
Which is (all numbers in microseconds):

Interval          |  start |     end
:---------------- | -----: | ------:
First rising edge |  0.000 |   0.417
Next falling edge |  4.120 |   4.536
Next rising edge  | 99.900 | 100.317

In this example, the detection was successful.

Configure a 1/1000 pulse for PWM channel #1:
```
$ rpio peripheral pwm data 1 1
```
Sample GPIO pin 18 (Pi-3):
```
$ rpio sample pulse 18 
0+4.17e-07 1.00e-04+4.16e-07 2.00e-04+4.17e-07
```
Which is (all numbers in microseconds):

Interval          |   start |     end
:---------------- |  -----: | ------:
First rising edge |   0.000 |   0.417
Next falling edge | 100.000 | 100.416 
Next rising edge  | 200.000 | 200.417

The detection failed. The implementation wasn't able to detect the signals's short 0.1µs-High-level period of the pulse. A "better" implementation (that makes use two pins) could watch for falling and rising edges at the same time, and hence wouldn't miss any (however, the accuracy of time points will still be an issue).

