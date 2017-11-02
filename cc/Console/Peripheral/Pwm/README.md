# Peripheral Pulse-Width-Modulation (PWM)

See [PWM](../../Rpi/Pwm.md) description in the library section.

## Synopsis

```
$ rpio peripheral pwm help
arguments: MODE [help]

MODE : berr       # raise BERR status flag
     | clear      # clear Status register
     | control    # set Control registers
     | data       # set Data register
     | dmac       # set DMA-Control register
     | fifo-cpu   # write FIFO by CPU
     | fifo-dma   # write FIFO by DMA
     | frequency  # estimate active transfer rate
     | range      # set Range register
     | status     # display register values
```

The clock rate needs to be set-up by the [clock-manager](../Cm).

### Status

There are no command line arguments.

Example:
```
# rpio peripheral pwm status

berr=0 empt=1 full=0 rerr=0 werr=0 DMA: enable=0 panic=7 dreq=7

# | gap sta | mode msen pola pwen rptl sbit usef |     data |     range
--+---------+------------------------------------+----------+----------
1 |   0   0 |    0    0    0    0    0    0    0 |        0 |       20
2 |   0   0 |    0    0    0    0    0    0    0 |        0 |       20
```

### Control Register

```
$ rpio peripheral pwm control help
arguments: OPTION+

clear             # clear FIFO
(+|-) mode (1|2)  # Serial or PWM mode
(+|-) msen (1|2)  # Mark-Space or coherent signal
(+|-) pwen (1|2)  # enable or disable transmission
(+|-) pola (1|2)  # inverse output polarity or don't
(+|-) rptl (1|2)  # repeat last word when idle or don't
(+|-) sbit (1|2)  # High or Low output when off
(+|-) usef (1|2)  # read FIFO or use Data register

E.g. -mode2 activates PWM mode for channel #2
```

The Control register is read, the options are applied and the value is written back. Hence, if the same flag is applied multiple times, only the last value sticks. I.e. if +pwen1 -pwen1, the Control register is written with PWEN1=0. 

### Control Register (DMA)

Set up DMA pacing:

```
$ rpio peripheral pwm dmac help
arguments: OPTION+

OPTION : enable BOOL
       | dreq     U8
       | panic    U8
```

### Frequency

Feed the FIFO and wait until the peripheral gets idle. The frequency results from the number of Words fed into the FIFO, multiplied by the number of configured bits per Word, divided by the measured time. Since PWM can be set-up with any clock-source, this is a simple way to determine the base frequency for each clock-source (see clock-manager).

```
$ rpio peripheral pwm frequency help
arguments: [-d DURATION]

DURATION: time in seconds to fill-up FIFO (default: 0.1)
```

For example:
```
$ rpio peripheral cm ctl pwm -enab +kill ;\
  rpio peripheral cm div pwm intgr 500 fract 0 ;\
  rpio peripheral cm ctl pwm mash 0 src 6 -kill ;\
  rpio peripheral cm ctl pwm +enab ;\
  rpio peripheral pwm control +mode1 +usef1 +pwen1 ;\
  rpio peripheral pwm status

berr=0 empt=1 full=0 rerr=0 werr=0 DMA: enable=0 panic=7 dreq=7

# | gap sta | mode msen pola pwen rptl sbit usef |     data |     range
--+---------+------------------------------------+----------+----------
1 |   0   0 |    1    0    0    1    0    0    1 |        0 |       20
2 |   0   0 |    0    0    0    0    0    0    0 |        0 |       20
  
$ rpio peripheral pwm frequency
1.00e+06 (3.13e+04,0)
```

The current PWM rate is 1 M/s which is 31.3 k words/s multiplied with 32 bits per word. There were no gaps (0).

### Fill FIFO (CPU)

Feed a binary file of 32-bit words by CPU into the FIFO. 

```
$ rpio peripheral pwm fifo-cpu help
arguments: [ -c U32 | -u ] FILE

-c  # detect underruns, this requires padding
-u  # unpaced write (ignores FIFO status)
default: fill FIFO whenver there is space

FILE = file with binary data to enqueue
```

There are three modes:
* In default mode, the FIFO is written whenever there is space.
* In "-c" mode, the FIFO is written in blocks of 16 words (if there is enough space).
* In "-u" mode, the FIFO is written regardless whether there is space or not.

The creation of a file with binary data is quite easy:
```
# seq 1 1 32 | while read ; do printf "\x00\x00\x00\x80" ; done > block_32
```
The file block_32 holds 32 words with the value 0x80000000.

Example (for a setup see the frequency example above):
```
$ rpio peripheral pwm status

berr=0 empt=1 full=0 rerr=0 werr=0 DMA: enable=0 panic=7 dreq=7

# | gap sta | mode msen pola pwen rptl sbit usef |     data |     range
--+---------+------------------------------------+----------+----------
1 |   0   1 |    1    0    0    1    0    0    1 |        0 |       20
2 |   0   0 |    0    0    0    0    0    0    0 |        0 |       20

$ rpio peripheral pwm fifo-cpu -u block_32 
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 1 0 1
```
The FIFO is written regardless whether there is space or not. This causes a write error (WERR=1).


### Fill FIFO (DMA)

Feed a binary file of 32-bit words by DMA into the FIFO. 

```
$ rpio peripheral pwm fifo-dma help
arguments: CHANNEL [CS] [TI] [ALLOC] FILE

CHANNEL = DMA channel (0..15)

CS = DMA Control and Status:
--cs[+-]disdebug
--cs=panic-priority 0..15
--cs=priority 0..15
--cs[+-]wait-for-outstanding-writes

TI = DMA Transfer Information:
--ti=burst-length 0..16
--ti[+-]dest-dreq
--ti[+-]dest-inc
--ti[+-]dest-ignore
--ti[+-]dest-width
--ti[+-]inten
--ti[+-]no-wide-bursts
--ti=permap 0..31
--ti[+-]src-dreq
--ti[+-]src-inc
--ti[+-]src-ignore
--ti[+-]src-width
--ti[+-]tdmode
--ti[+-]wait-resp
--ti=waits 0..31

ALLOC = allocator for DMA bus memory:
ALLOC : --memf arm [COHERENCY] [-s]
      | --memf gpu [-a ALIGN] [-m MODE] [-d COHERENCY]

COHERENCY:
[--co-[0|4|8|c]] (default --co-4)

ALIGN = align the memory segment (U32)

MODE
bit:0: discardable
bit:2: direct allocation
bit:3: coherent allocation
bit:4: fill with zeros
bit:5: don't initialize
bit:6: lock permenently

-d: don't use /dev/vcio

FILE = file with binary data to enqueue
```

Example: Set a strip of 30x WS2812B LEDs to the brightest value. The data to send consists of a 50us Low-signal as reset, 30x24 *bits* each represented by a 1-1-0 sequence at 400ns, and another 50us Low-signal. 

The binary file is create by shell and AWK command:
```
$ ( \
for i in `seq 1 1 125` ; do echo -n "00 " ; done ; \
for i in `seq 1 1 90` ; do echo -n "DB 6D B6 " ; done ; \
for i in `seq 1 1 125` ; do echo -n "00 " ; done \
) | \
awk '{for (i=1;i<=NF;i+=4) print $(i+3),$(i+2),$(i+1),$(i)}' |\
while read a b c d ; do printf "\x$a\x$b\x$c\x$d" ; done > ws.on
```

That is:
```
$ od -Ad -txC ws.on
0000000 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*
0000112 00 00 00 00 00 00 00 00 00 00 00 00 b6 6d db 00
0000128 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000144 6d db b6 6d b6 6d db b6 db b6 6d db 6d db b6 6d
0000160 b6 6d db b6 db b6 6d db 6d db b6 6d b6 6d db b6
0000176 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000192 6d db b6 6d b6 6d db b6 db b6 6d db 6d db b6 6d
0000208 b6 6d db b6 db b6 6d db 6d db b6 6d b6 6d db b6
0000224 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000240 6d db b6 6d b6 6d db b6 db b6 6d db 6d db b6 6d
0000256 b6 6d db b6 db b6 6d db 6d db b6 6d b6 6d db b6
0000272 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000288 6d db b6 6d b6 6d db b6 db b6 6d db 6d db b6 6d
0000304 b6 6d db b6 db b6 6d db 6d db b6 6d b6 6d db b6
0000320 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000336 6d db b6 6d b6 6d db b6 db b6 6d db 6d db b6 6d
0000352 b6 6d db b6 db b6 6d db 6d db b6 6d b6 6d db b6
0000368 db b6 6d db 6d db b6 6d b6 6d db b6 db b6 6d db
0000384 6d db b6 6d b6 6d db b6 00 b6 6d db 00 00 00 00
0000400 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
*
0000512 00 00 00 00 00 00 00 00
0000520
```

Set up PWM at 2.5 M/s (400 ns) for channel #1:
```
$ rpio peripheral cm set pwm -f 0 -i 200 -s 6 ;\
  rpio peripheral cm switch pwm on ;\
  rpio peripheral pwm control +mode1 +usef1 +pwen1 ;\
  rpio peripheral pwm dmac enable 1 ;\
  rpio peripheral gpio mode 12 0

$ rpio peripheral pwm status

berr=0 empt=1 full=0 rerr=0 werr=1 DMA: enable=1 panic=7 dreq=7

# | gap sta | mode msen pola pwen rptl sbit usef |     data |     range
--+---------+------------------------------------+----------+----------
1 |   0   0 |    1    0    0    1    0    0    1 |        0 |       20
2 |   0   0 |    0    0    0    0    0    0    0 |        0 |       20
  
$ rpio peripheral pwm frequency
2.50e+06 (7.81e+04,1)
```

Switch the LEDs on:
```
$ rpio peripheral pwm fifo-dma 0 ws.on
```

The file generated below can be used to switch the lights off.
```
$ ( \
for i in `seq 1 1 125` ; do echo -n "00 " ; done ;  \
for i in `seq 1 1 90` ; do echo -n "24 92 49 " ; done ; \
for i in `seq 1 1 125` ; do echo -n "00 " ; done \
) | \
awk '{for (i=1;i<=NF;i+=4) print $(i+3),$(i+2),$(i+1),$(i)}' |\
while read a b c d ; do printf "\x$a\x$b\x$c\x$d" ; done > ws.off
```

