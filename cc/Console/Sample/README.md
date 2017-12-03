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

The Raspberry Pi uses several clocks to drive its bus systems (see [eLinux](https://elinux.org/RPiconfig)).

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

## Considerations

### Signal Propagation

There is a delay between the signal setup at the source and the signal setting at the test point where the signal is read. If a parallel interface is used, the signal propagation among the parallel wires will vary since signal propagation depends (besides others) on the lengths of the wires and the PCB traces.

As an example, a clock pulse is generated at one pin and read back on ten other pins; which have to beare (manually) hot-wired to the signal pin.

Here we use GPIO pin 4 (GP0) to generate the clock-pulse. GPIO pins 5 to 14 are used to read the signal:
```
$ rpio peripheral gpio mode 4 0 && \
  rpio peripheral gpio mode -m 0x7fe0 i
```

GP0 is set to 5 MHz:
```
$ rpio peripheral cm ctl gp0 -enab +kill && \
  rpio peripheral cm div gp0 intgr 100 fract 0 && \
  rpio peripheral cm ctl gp0 mash 0 src 6 -kill && \
  rpio peripheral cm ctl gp0 +enab
```

Data is sampled; thereafter saved to the file "10mps.bin":
```
$ rpio sample level buffer 10000000 10mps.bin
```

Each word in the file holds the voltage levels of GPIO pins 0-31. 
```
$ od -v -tx -w4 10mps.bin | head
0000000 3000ffff
0000004 3000800f
0000010 3000ffff
0000014 3000800f
0000020 3000800f
0000024 3000ffff
0000030 3000800f
0000034 3000800f
0000040 3000ffff
0000044 3000800f
```

Bit 4 is the GP0's clock signal; bits 5 to 14 are hot-wired to pin 4 and hence "should" show the same value as GP0. Let's see what values we actually have got:
```
$ od -v -tx -w4 10mps.bin |\
  awk 'NF==2 {N[$2]++} END {for (i in N) print i,N[i]}' |\
  sort
3000800f 4535388
3000801f 135286
3000821f 23186
3000861f 34722
30008e1f 1618
3000c01f 77324
3000c41f 18481
3000c61f 10837
3000ce1f 37771
3000cf1f 7000
3000cf9f 23685
3000cfdf 4418
3000dfdf 927
3000dfff 2434
3000ee1f 22795
3000ef1f 454
3000fe1f 470
3000ff1f 4282
3000ff5f 995
3000ff7f 1007
3000ffef 113622
3000ffff 4943298
```
The left column shows the pins' voltage level. The right column the number of occurences.

In most cases we've got either the word 0x3000800f (pins 4-14 are all low) or the word 0x3000ffff (pins 4-14 are all high). However, there is also a significant number of cases with other values, i.e. pins 4-14 are not all low/high.

So we learn that the signal propagation has to be taken into account when analyzing conjugate signals on parallel paths. For example, many protocols provide a clock pulse and the sender sets the signal line(s) together with the clock's edge; then, the subsequent clock edge can be used as indicator that the signal line(s) become valid for the reader.

### Sample Rate

Let's poll the GPIO-pin-levels in a busy loop in Linux userland. We will also take the time before the first sample and after the last sample: This enables us to calculate the average sample rate. The reciprocal of the sample rate gives us the average time between two subsequent samples.

The average time between two samples does not necessarily reflect the actual time between (any) two samples. For example, imagine 1,000 samples were taken within a second. That is one sample each millisecond. However, it might be quite possible that all the samples were taken within 500ms, and then (for whatever reason) the sampling stopped for the rest of the time.

On many architectures and especially in Linux userland there is no guarantee that instructions are executed at steady intervals. Hence we're going to perform a little experiment with an external counter: We use a 10-bit ripple counter which is assembled by a (fast) 74AC74 circuit (2x D-FF) and a (slow) 74HC393 circuit (2x 4-Bit Counter). Such a counter can be used at a clock pulse of something about 50 MHz. You can buy these circuits for less than a buck. 

We use GPIO pin 4 (GP0) to generate the clock-pulse for the D-FF. It is connected to the 2nd D-FF, which in turn to the first 4-bit counter, which in turn to the 2nd 4-bit counter. GPIO pins 5 to 14 are used to read the resulting 10-bit-counter.
```
$ rpio peripheral gpio mode 4 0 && \
  rpio peripheral gpio mode -m 0x7fe0 i
```

GP0 is set to 50 MHz which gives us 10 ns Low and High levels each in turn on GPIO pin 4:
```
$ rpio peripheral cm ctl gp0 -enab +kill && \
  rpio peripheral cm div gp0 intgr 10 fract 0 && \
  rpio peripheral cm ctl gp0 mash 0 src 6 -kill && \
  rpio peripheral cm ctl gp0 +enab
```

We sample the data into RAM, then save the sampled data to file (100mps.bin):
```
$ rpio sample level buffer 10000000 100mps.bin
r=1.61e+07/s
```
The average sample rate is 16.1M/s. That is an average delay of 1/r=62ns between subsequent samples.

A single counter increment corresponds to 10ns (because of the 50 MHz clock). Since the measured average sample rate is 62ns, we should see something of a 6-point increment for subsequent samples.

Note: Od, Awk and Perl are used below to fetch bits 4 to 14 and display only those as a decimal and as binary value. We inverse Bit-4 since the D-FF uses a rising edge for increments. 

Let's have a look at the sampled values (decimal left column, binary right column):
```
$ od -v -tu -w4 100mps.bin | awk '{print $2}' |\
  perl -ne 'my $v=1^(($_/16)%0x800) ; printf "%4u %11b\n",$v,$v' |\
  head
1632 11001100000
1657 11001111001
1659 11001111011
1669 11010000101
1676 11010001100
1682 11010010010
1680 11010010000
1694 11010011110
1697 11010100001
1707 11010101011
```
Well, this doesn't look as uniform as hoped for; which doesn't come as complete suprise since we're using a [ripple counter](https://en.wikipedia.org/wiki/Counter_(digital)#Asynchronous_.28ripple.29_counter): It takes some time until a carry-over ripples through all the bits. Thus it seems as if the counter counts backwards at certain points (e.g. from 1682 to 1680).

Let's have a closer look at these ripples; this time with a 5 MHz clock:
```
$ rpio peripheral cm ctl gp0 -enab +kill && \
  rpio peripheral cm div gp0 intgr 100 fract 0 && \
  rpio peripheral cm ctl gp0 mash 0 src 6 -kill && \
  rpio peripheral cm ctl gp0 +enab

$ rpio sample level buffer 10000000 10mps.bin
```

The ripple is best seen when the counter wraps around. The least sigificant bit changes from 1 to 0 and causes a ripple through all bits:
```
$ od -v -tu -w4 10mps.bin | awk '{print $2}' |\
  perl -ne 'my $v=1^(($_/16)%0x800) ; printf "%4u %11b\n",$v,$v' |\
  grep -A4 '11111111111$' | head
...
2047 11111111111
2040 11111111000
1792 11100000000
   1           1
...   
```

Whenever there is an (invisible) ripple in progress, it gets quite difficult to work with such a sample. That becomes even harder if there are several ripples in progress: At a faster clock rate, the lower bits may have been incremented a few times before the ripple through the higher bits completed. Below an example (@ 100 M/s):
```
$ od -v -tu -w4 100mps.bin | awk '{print $2}' |\
  perl -ne 'my $v=1^(($_/16)%0x800) ; printf "%4u %11b\n",$v,$v' |\
  grep -A4 '11111111111$' | head
...
2047 11111111111
2018 11111100010
   0           0
  14        1110
...  
```
The sample in the second row reads as 2 while the ripple isn't completed yet. Even more annoying, there is probably a second ripple in progress within the lower bits. It reads as 2. However, there is probably transition from 3=0b11 to 4=0b100 in progress.

The ripple in the high bits is completed in the 3rd row. However, again, there is probably a ripple from 7=0b111 to 8=b1000 in progress.

The value in the 4th row could be without any ripple in progress. But it is also possible that a ripple from 15=0b1111 to 16=b10000 is in progress.

That is kind of annoying. Luckily there are certain values which are a safe bet: there can't be any ripple in progress for these values. Firstly, if all bits read as 1 then no ripple is in progress.

Second, there is a guarantee of a maximum time a propagation make take (from the clock signal to the highest bit). If we clock the counter for this duration w/o causing another ripple, we've got also a correct counter value.

For example, if the maximum propagation delay is 100ns and if the counter gets incremented every 10ns (at a 50 MHz clock) then we need to wait for 10 cycles. This means that all samples that end in 0xf are correct since 0xf is above 10 and since 0xf has all its bits set (so we're sure there is no new ripple in progress).

Below you see a Perl script that prints each sample (decimal + binary). If the sample ends in 0xf the script does also print the average number of clocks-per-sample since the last correct counter value. The 4th value is the number of samples since the last correct counter value.
```
#!/usr/bin/perl

use strict ;
use warnings ;

my $c = 0 ;
my ($lastC,$lastV) ;

my $buffer ;
my $n = read(\*STDIN,$buffer,4) ;

while (defined($n) && $n == 4)
{
    my $v = ord(substr($buffer,3,1)) ;
    {
	use integer ;
	$v <<= 8 ;
	$v += ord(substr($buffer,2,1)) ;
	$v <<= 8 ;
	$v += ord(substr($buffer,1,1)) ;
	$v <<= 8 ;
	$v += ord(substr($buffer,0,1)) ;
    }

    $v = 1 ^ (($v/16) % 0x800) ;
    printf "%4u %11b ",$v,$v ;
    
    if (defined($lastV) && $v == $lastV)
    {
    }
    elsif (0xf == ($v % 0x10))
    {
	if (defined($lastV))
	{
	    $lastV -= 0x800 
		if $v < $lastV ;
	    my $range = $c - $lastC ;
	    printf "%.03f %u", ($v-$lastV)/$range, $range ;
	}
	$lastC = $c ; $lastV = $v ;
    }
    printf "\n" ;

    $n = read(\*STDIN,$buffer,4) ;
    ++$c ;
}

die("file not a multiple of word-size")
    if !defined($n) || ($n != 0) ;
```

The script reads the binary file with the samples from STDIN. The output may look like this (50 MHz clock):
```
1407 10101111111 6.400 30
1344 10101000000 
1408 10110000000 
1422 10110001110 
1428 10110010100 
1434 10110011010 
1437 10110011101 
1447 10110100111 
1449 10110101001 
1451 10110101011 
1462 10110110110 
1468 10110111100 
1458 10110110010 
1472 10111000000 
1487 10111001111 5.714 14
1489 10111010001 
1491 10111010011 
1500 10111011100 
1508 10111100100 
1514 10111101010 
1512 10111101000 
1526 10111110110 
1529 10111111001 
1531 10111111011 
1541 11000000101 
1551 11000001111 5.818 11
```

We're only interested in the correct counter values. Let's see what we've got:
```
$ cat 100mps.bin | script.pl |\
  awk 'NF==4 {N[int($3+.5)]+=$4} END {for (i in N) print i,N[i]}' |\
  sort -n
5 54
6 7810722
7 1746706
8 201781
9 65070
10 133683
11 18318
12 8874
13 2251
14 2743
15 445
16 3427
17 372
18 297
19 427
20 1153
...
214 8
236 8
348 4
411 3
```
The left column shows the average number of clocks per sample (each clock pulse is 10ns). The right column shows the number of samples that were taken at this rate.

So, 78 percent (7,810,722) of the sample are taken at an average interval of ~60ns (6x 10ns), another 17.5% at ~70ns. However, there are many samples that were taken far beyond the average interval of 62ns.

In conclusion, if an application requires fixed intervals to sample; or at least intervals that don't exceed a pre-defined minimum duration; then CPU sampling won't provide the means to achive this goal. This isn't surprising, considering how Linux userland applications work. However, what does a little surprise, is the wide range of intervals.

The approach presented here is only suitable to disprove the theory that fixed or minimum intervals between samples are achievable. The contrary is not true. Even if the output above would only show intervals of 60ns, actual samples might deviate since we've calculated an average for counters that end in 0xf. Besides, wrap-arounds are not accounted for.
