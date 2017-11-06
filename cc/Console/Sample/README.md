# Sample Data

Note: the executing thread may be interrupted and suspended at any time for any duration by the operating system. Hence, the results of the program should be considered with care.

## Synopsis

```
$ ./rpio sample [help]
```

## Sample Input Level

The input level register (GPLEV0) is polled in a busy loop. The program expects the number of iterations and the GPIO pin to watch. There are two counters: One counter increments with each detected High level input. The other counter increments with each Level change (from High to Low and Low to High).

The program outputs:
* The sample rate (r) which is number of iterations per second.
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
$ rpio sample duty 18 100000000
r=1.65e+07/s f=9.98e+05/s duty=1.00e-01
```
The sample rate was 16.5 M/s, the determined frequency 0.998 MHz (instead of 1 MHz) and the determined duty cycle 0.1.

Configure a 7/10 pulse for PWM channel #1:
```
$ rpio peripheral pwm data 1 7
```

Sample GPIO pin 18 (Pi-3):
```
$ rpio sample duty 18 100000000
r=1.65e+07/s f=9.99e+05/s duty=7.01e-01
```
The sample rate was 16.5 M/s, the determined frequency 0.999 MHz (instead of 1 MHz) and the determined duty cycle 0.701 (instead of 0.7).

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

Clearly, the detection failed. The implementation wasn't able to detect the signals's short 0.1µs-High-level period of the pulse. A "better" implementation (that makes use two pins) could watch for falling and rising edges at the same time, and hence wouldn't miss any (however, the accuracy of time points will still be an issue).