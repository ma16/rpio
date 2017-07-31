# Peripheral Pulse-Width-Modulation (PWM)

See ../../Rpi/Pwm.md

## Synopsis

```
$ ./rpio pwm help
arguments: MODE [help]

MODE : control    # write peripheral registers
     | dma        # send data in DMA/FIFO mode
     | dummy      # send dummy data in DMA/FIFO mode
     | frequency  # estimate current frequency
     | send       # send data in CPU/FIFO mode
     | status     # display status
```

The clock-pulse needs to be set-up by the clock-generator beforhand.

### Status

There are no command line arguments.

Example:

```
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

sta2 sta1 berr gap2 gap1 rerr werr empt full
--------------------------------------------
   0    0    0    0    0    0    0    1    0 (0x2)

# msen usef pola sbit rptl mode pwen     data    range
------------------------------------------------------
0    0    1    0    0    0    1    0        0       20
1    0    0    0    0    0    0    0        0       20
```

### Control

```
$ ./rpio pwm control help
arguments: COMMAND+

General commands:
clear         # clear FIFO
dma   BOOL    # enable/disable DMA signals
dreq    U8    # threshold for DMA DREQ signal
panic   U8    # threshold for DMA PANIC signal
pwen  BOOL    # start (1) or stop (0) both channels
send   U32    # enqueue word in FIFO
reset         # reset status flags

Channel-specific commands:
data.CH  U32  # set data register
mode.CH BOOL  # serialize (1) or PWM (0)
msen.CH BOOL  # enable M/S (only for PWM-mode)
pwen.CH BOOL  # start (1) or stop (0)
pola.CH BOOL  # inverse output polarity
range.CH U32  # set range register
rptl.CH BOOL  # repeat last data when FIFO is empty
sbit.CH BOOL  # silence-bit for gaps
usef.CH BOOL  # use FIFO
CH must be either 0 or 1, e.g. pola.1 for channel 1
```

### Frequency

Feed the FIFO and wait until the serializer gets idle. The frequency results from the number of Words fed into the FIFO, multiplied by the number of configured bits per Word, divided by the measured time. Since PWM can be set-p by any clock-source, this is a simple way to determine the base frequency of each clock-source (see clock-manager).



### Send

Feed a binary file of 32-bit words into the FIFO. A proper set-up is required beforhand.

```
$ ./rpio pwm send help
arguments: INDEX FILE

INDEX = channel to use (0,1)
 FILE = name of file with data to transfer

you may want to set up the registers beforehand
```

### Send (Example)

Set a stripe of 30x WS2812B LEDs to the brightest value.

The data to send consists of a 50us-Low signal to reset, 30x24 *bits* representing each a *1-bit*, and another 50us-Low signal. Each *1-bit* is a H-H-L signal, where each level lasts for 400ns. Hence we apply a frequency of 2.5 MHz.

The file is create by shell and AWK command:
```
$ ( \
for i in `seq 1 1 125` ; do echo -n "00 " ; done ;  \
for i in `seq 1 1 90` ; do echo -n "DB 6D B6 " ; done ; \
for i in `seq 1 1 125` ; do echo -n "00 " ; done \
) | \
awk '{for (i=1;i<=NF;i+=4) print $(i+3),$(i+2),$(i+1),$(i)}' |\
while read a b c d ; do printf "\x$a\x$b\x$c\x$d" ; done > ws.on
```

Let's have a look at the generated file (note that Pi uses little endian):
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

We use channel 0. If properly set-up and connected, the stripe should turn on with:
```
$ ./rpio pwm send 0 ws.on
```

To switch off the lights, we use 30x24 *0-bits* as H-L-L signal:
```
$ ( \
for i in `seq 1 1 125` ; do echo -n "00 " ; done ;  \
for i in `seq 1 1 90` ; do echo -n "24 92 49 " ; done ; \
for i in `seq 1 1 125` ; do echo -n "00 " ; done \
) | \
awk '{for (i=1;i<=NF;i+=4) print $(i+3),$(i+2),$(i+1),$(i)}' |\
while read a b c d ; do printf "\x$a\x$b\x$c\x$d" ; done > ws.off
```

The stripe should turn off with:
```
$ ./rpio pwm send 0 ws.off
```

