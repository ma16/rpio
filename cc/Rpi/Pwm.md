# Pulse Width Modulator (PWM)

See [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf): Pulse Width Modulator (§9). Please refer also to the errata section at the end of this page.

Highlights:
* Two output channels.
* Operates also as generic serializer.
* 16-word deep FIFO with 32-bit words.
* DMA pacing.
* Clock-manager.

## Clock Rate

Clock source and pre-scaler are configured by the [clock-manager](../Console/Cm) peripheral (CM):
* The Control register is CM_PWMCTL at address 7E10:10A0.
* The Divider register is CM_PWMDIV at address 7E10:10A4.

This enables effective clock-rates in a range of about 5 kHz to 125 MHz.

## Register Block

The base-address of the register-block is 7E20:C000.

Offset | Name | Abstract | Channel
---: | :--- | :--- | ---:
0x0 | CTL | Control | both
0x4 | STA | Status | both
0x8 | DMAC | DMA-Control | -
0x10 | RNG1 | Data-range as bits per word | #1
0x14 | DAT1 | 32-bit data-word | #1
0x18 | FIF1 | FIFO | -
0x20 | RNG2 | Data-range as bits per word | #2
0x24 | DAT2 | 32-bit data-word | #2

The registers are described below. The range registers RNG1 and RNG2 are described in the RNG# section and the data registers DAT1 and DAT2 are described in the DAT# section.

## Control Register (CTL)

### Overview

Offset | Name | Abstract | Channel
-----: | :--- | :------- | ------:
0 | PWEN1 | Enable Transmission | #1
1 | MODE1 | Mode | #1
2 | RPTL1 | Repeat-Last-Word when idle | #1
3 | SBIT1 | Silent-Bit when idle | #1
4 | POLA1 | Inverse Polarity | #1
5 | USEF1 | Use FIFO instead of Data register | #1
6 | CLRF1 | Clear FIFO | -
7 | MSEN1 | Mark-Space-Enable | #1
8 | PWEN2 | Enable Transmission | #2
9 | MODE2 | Mode | #2
10 | RPTL2 | Repeat-Last-Word when idle | #2
11 | SBIT2 | Silent-Bit when idle | #2
12 | POLA2 | Inverse Polarity | #2
13 | USEF2 | Use FIFO instead of Data register | #2
15 | MSEN2 | Mark-Space-Enable | #2

Name | Description
:--- | :----------
CLRF1 | Write 1 to clear the FIFO. Writing 0 has no effect. Read returns always zero.
MODE# | 0 = PWM, 1 = Serial; see sections below
MSEN# | 0 = coherent, 1 = mark-space; see sections below; only effective if MODE=0
POLA# | 0 = normal, 1 = inverse output polarity
PWEN# |	0 = disable, 1 = enable transmission 
RPTL# | 0 = normal, 1 = repeat last-sent word if idle and don't set the GAP Status flag; only effective if USEF=1
SBIT# | 0 = Low, 1 = High output if idle
USEF# | 0 = use Data register, 1 = use FIFO instead; see sections below

**RPTL**

This flag defines the behavior of the peripheral when the FIFO runs empty.

If RPTL=0, the peripheral will stop the transmission. GAP will be set and STA will be cleared. The output signal will be as configured in CONTROL.SBIT. When a new word is put into the FIFO, the transmission will continue with this word (STA=1).

If RPTL=1, the peripheral will continue transmission and repeat the last word.  GAP won't be raised and STA remains set. When a new word is put into the FIFO, the transmission will continue with this word.

It appears that channel #1 operates always as if RPTL1=1 (see Defect section). This makes the Status flags STA1 and GAP1 ineffective.

### PWM Mode

In PWM mode (MODE=0) the output signal is defined by the period and by the ratio:

* The *period* (P) is the duration of a single PWM-cylce that repeats again and again. It is given as a number of clock-pulses.
* The *ratio* (R) reflects the average strength of the output signal. It is given as as the number of clock-pulses (within a PWM-cycle) when the output-level is High (R<=P).

R | Output Signal | Description
---: | ---: | :---
0 | 0 | Permanently Low
1 | 1 / P | Lowest sensible value above Low
P-1 | (P-1) / P | Highest sensible value below High
P | 1 | Permanently High

In *mark-space* operation (M/S), the output is set to Low for (*P*-*R*) clock-pulses and then to High for *R* clock-pulses.

A *coherent* operation is also supported which spreads the Lows and Highs (within a cycle) evenly. For example: (*R*,*P*) = (3,10)

```
       M/S cycle: 0 0 0 0 0 0 0 1 1 1
  coherent cycle: 0 1 0 0 1 0 0 1 0 0
```

### Serial Mode

If Serial mode (MODE=1) is used instead of PWM, the output signal corresponds to the bits of a given word. Most significant bits are put out first. This enables arbitrary bit-stream of almost any length in FIFO mode (USEF=1).

## Status Register (STA)

Offset | Name | Abstract | Channel | Clear
-----: | :--- | :------- | ------: | :----
0 | FULL | FIFO is full | - | -
1 | EMPT | FIFO is empyt | - | -
2 | WERR | A write to the FIFO failed (full) | - | ✓
3 | RERR | A read from the FIFO failed (empty) | - | ✓
4 | GAP1 | FIFO underrun | #1 | ✓
5 | GAP2 | FIFO underrun | #2 | ✓
8 | BERR | Bus Error | - | ✓
9 | STA1 | Channel is currently transmitting data | #1 | -
10 | STA2 | Channel is currently transmitting data | #2 | -

A bit remains set until cleared. Write 1 to clear a bit. Write 0 has no effect.

The flags GAP1 and STA1 are ineffective since peripheral acts as if the Control flag RPTL1 is always set (see Defects).

**BERR**

An error has occurred while writing to registers via APB. This may happen if the bus tries to write successively to the same set of registers faster than the synchronizer block can cope with. Multiple switching may occur and contaminate the data during synchronisation.

This kind of problem can be observed when writing twice in a row to the Control register (Use-Case-6). Since the effects are unpredictable, application developers should check for BERR after and try to prevent it. Adding additional read-cyles may be used as **work-around**. Still, this remains an open issue.

Note that the BERR flag appears always to be raised when the Control register is written but the clock-manager wasn't set-up yet (CM.PWM.CONTROL.ENAB=0).

**RERR**

There is no explanation under which circumstances this flag ist set.

Reading the FIFO doesn't raise the flag (Use-Case-7).

## FIFO Register (FIF1)

A write operation to this register appends a word onto the FIFO. The FIFO itself is 16 words deep. Each word is 32-bit wide. The FIFO can only be written by an application, it can not be read. The status of the FIFO is reflected by STA.EMPT, STA.FULL, STA.RERR and STA.WERR.

When writing the register you should make sure: 
* either beforhand that the FIFO is not full
* or afterwards that no write-error has occurred.

The FIFO is shared between both channels. Hence, when both channels are enabled for FIFO usage:
* The data is shared between these channels in turn. For example, with the word sequence A B C D E F G H, the first channel will use A C E G and the second channel will use B D F H.
* The range register should hold the same value for both channels.
* The datasheet says that RPTL# is not meaningful as there is no defined channel to own the last data in the FIFO. Therefore both RPTL# flags must be set to zero. However, observations show, it doesn't has to.
* If the configuration has changed in any of the two channels, the FIFO should be cleared before writing new data.

### FIFO Underrun

A FIFO underrun in Serial mode will distort the transferred data. This might not be acceptable for many applications. Still, it cannot be prevented. At least not for data that exceeds the FIFO buffer.

In a DMA mode, a FIFO underrun is less likely. However, in CPU mode, the operating thread can be suspended at any time for any duration. At the very least it might be quite helpful to detect a FIFO underrun. So, this kind of information is provided by the GAP Status flag (Use-Case-8).

The GAP flag is not raised in RPTL mode. If data is transferred on channel #1, the GAP1 flag is never raised since channel #1 operates as if RPTL1=1 (see defect list).

## Range Register (RNG#)

In PWM mode (CTL.MODE#=0) the range defines the duration of a period; by the number of corresponding clock-cycles.

In Serial mode (CTL.MODE#=1) the range defines the number of bits in a word to transmit:
* If range < 32, only the most significant bits of a word are transmitted. The remaining (least signficant) bits in the word are ignored.
* If range > 32, the word is filled-up with (least significant) padding bits.

Note: in Serial mode (CTL.MODE#=1) with FIFO (CTL.USEF#=1) two bits are transferred per word even if RNG#=1. If RNG#=0 there are strange effects, i.e. a single write to an empty FIFO enables STA.STA# (if CTL.PWEN#=1), but the FIFO remains empty. Two writes in a row make the FIFO non-empty.

## Data Register (DAT#)

This register is only relevant if CTL.USEF#=0.

In PWM mode (CTL.MODE#=0) the value effects the ratio. The register defines the number of clock-cycles within a period (RNG#) the output should be set to High.

In Serial mode (CTL.MODE#=1) the value defines the data to transmit (again and again).

## DMA-Control Register (DMAC)

The peripheral mapping (TI.PERMAP) to pace DMA write operations to the PWM-FIFO is 5.

Offset | Size | Name | Abstract | Default
-----: | ---: | :--- | :------- | ------:
0 | 8 | DREQ | Threshold when to put DREQ signal on the bus | 7
8 | 8 | PANIC | Threshold when to put PANIC signal on the bus | 7
31 | 1 | ENAB | Enable DMA pacing, i.e. DREQ and PANIC signals | -

That is: the DREQ signal is raised on the AXI bus when the number of entries in the FIFO drops to DMAC.DREQ. This signal tells the DMA controller to resume the transfer (and stop if the signal is cleared).

 The value of DMAC.DREQ needs to be chosen carefully: there will be a FIFO underrun if DREQ is too small; and there will be a FIFO overflow if DREQ is too big.

From observation: DMAC.DREQ > 9 leads to a FIFO overflow. The DMA controller doesn't stop the transfer in time, so data is still written when the FIFO is already full.

DREQ |0..15|  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  24 |  25 |  26 |  27 |  28 |  29|  30 |  31
---: | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- 
 9 | 0..15|16|17|18|19|20|21|22|23|24|25|26|27|28|29|30|31
10 | 0..15| *|17|18|19|20|21|22|23| *|25|26|27|28|29|30|31
11 | 0..15| *| *|18|19|20|21|22|23| *| *|26|27|28|29|30|31
12 | 0..15| *| *|18|19|20|21|22| *| *| *|26|27|28|29|30| *
13 | 0..15| *| *|18|19|20|21| *| *| *| *|26|27|28|29| *| *
14 | 0..15| *| *|18|19|20| *| *| *| *| *|26|27|28| *| *| *
15 | 0..15| *| *|18|19| *| *| *| *| *| *|26|27| *| *| *| *

So there are missing values (*) because of writing into a full FIFO; this appears to be reproducible at different speeds. 

The overflow won't happen if the DMA controller uses TI.WAIT_RESP=1. This makes the DMA controller wait after each write until it receives the AXI write-response. It ensures that multiple writes cannot get stacked in the AXI bus pipeline. The maximum transfer rate will drop accordingly.

## Defects

### No Transmission on Channel #2

Even though the channel is active, the peripheral won't transmit a newly enqueued word (Use-Case-1). The control test on channel #1 succeeds however (Use-Case-2).

There is a slight difference between these two test cases: RPTL1 is implicitly set for channel #1. However, setting RPTL2 to imitate this behavior doesn't make any difference (Use-Case-3).

**Work-around**: The FIFO should be cleared whenever channel #2 is used (Use-Case-4).

### CONTROL.RPTL1

The peripheral behaves always as if RPTL1=1, whether the flag is actually set or not (Use-Case-2). This is corroborated by the Status flags since STA1=1 and GAP1=0. A logic analyzer shows the continuous repetition of the signal.

The control test is performed on channel #2 (Use-Case-4). The RPTL1 flag is not set. A logic analyzer shwows only a single occurrance of the signal; without any repetitions. Since there is no more data in the FIFO, the transmission stopped (STA2=0, GAP2=1).

Note: There is a difference between an explicitly set RPTL2 flag and the implicitly set RPTL1 flag. On start-up, channel #1 is not transmitting any signal; however, channel #2 is if RPTL2=1 (compare Use-Case-2 and Use-Case-3).

## Use Cases

All use-cases were run on a Pi model-0 (BCM2835), on model-2 (BCM2836) and on model-3 (BCM2837). Raspbian was just booted up unless otherwise mentioned.

### Use-Case-1

Put a single word into the FIFO for transmission on the active channel #2.

```
$ ./rpio cm set pwm -f 0 -i 200 -s 6
$ ./rpio cm switch pwm on
$ ./rpio pwm control usef2=1 mode2=1 pwen2=1
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0       20
$ ./rpio gpio mode 13 0
$ ./rpio pwm enqueue 0xf0555500
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    0    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0       20
```

Observe: The word remains in the FIFO. No transmission took place.

### Use-Case-2

Put a single word into the FIFO for transmission on the active channel #1.

```
$ ./rpio cm set pwm -f 0 -i 200 -s 6
$ ./rpio cm switch pwm on
$ ./rpio pwm control usef1=1 mode1=1 pwen1=1
$ ./rpio pwm status
berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    1    0    0    0    1    1        0       20
2   0   0    0    0    0    0    0    0    0        0       20
$ ./rpio gpio mode 12 0
$ ./rpio pwm enqueue 0xf0555500
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   1   0    0    1    0    0    0    1    1        0       20
2   0   0    0    0    0    0    0    0    0        0       20
```

Observe: The peripheral repeats the transmission of the word even though RPTL1 is not set.

### Use-Case-3

Put a single word into the FIFO for transmission on the active channel #2. RPTL2 is set.

```
$ ./rpio cm set pwm -f 0 -i 200 -s 6
$ ./rpio cm switch pwm on
$ ./rpio pwm control usef2=1 mode2=1 rptl2=1 pwen2=1
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   1   0    0    1    0    0    1    1    1        0       20
$ ./rpio gpio mode 13 0
$ ./rpio pwm enqueue 0xf0555500
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    0    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   1   1    0    1    0    0    1    1    1        0       20
```

Observe: The word remains in the FIFO. No transmission took place even though STA2=1.

### Use-Case-4

Put a single word into the FIFO for transmission on the active channel #2. The FIFO is cleared beforhand.

```
$ ./rpio cm set pwm -f 0 -i 200 -s 6
$ ./rpio cm switch pwm on
$ ./rpio pwm control usef2=1 mode2=1 pwen2=1
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0       20
$ ./rpio gpio mode 13 0
$ ./rpio pwm control clear
$ ./rpio pwm enqueue 0xf0555500
# ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   1    0    1    0    0    0    1    1        0       20
```

Observe: The transmission is stopped (STA=1) and the GAP flag is raised since there is no more data in the FIFO.

### Use-Case-5

This continues Use-Case-4. Activate RPTL and deactivate again.

```
$ ./rpio pwm clear gap2
$ ./rpio pwm control rptl2=1
# ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   1   0    0    1    0    0    1    1    1        0       20
$ ./rpio pwm control rptl2=0
# ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   1    0    1    0    0    0    1    1        0       20
```

Observe: The transmission is continued (STA=1) when RPTL is set and stopped (STA=0) again if reset.

### Use-Case-6

Read the Control word N times and write it back. This is done in a loop with 100 repetitions.

The CM has to be set-up beforhand. Otherwise the BERR flag is always set.

```
$ ./rpio pwm clear berr
$ ./rpio pwm berr 5
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   1    0    0    0    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
1   0   0    0    0    0    0    0    0    0        0       20
$ ./rpio pwm clear berr
$ ./rpio pwm berr 20
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   1    0    0    0    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
1   0   0    0    0    0    0    0    0    0        0       20
```

Observe: The BERR Status flag is raised if the Control register is written consecutively in a fast manner (N=5); and not raised if there is some delay (N=20).

### Use-Case-7

Read the FIFO when it is empty.

```
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

sta2 sta1 berr gap2 gap1 rerr werr empt full
--------------------------------------------
   0    0    0    0    0    0    0    1    0 (0x2)

# msen usef pola sbit rptl mode pwen     data    range
------------------------------------------------------
1    0    1    0    0    0    1    0        0       20
2    0    0    0    0    0    0    0        0       20
$ ./rpio poke -p 0x20c018 get
70776d30
```

Observe: Reading returns 0x70776d30 which spells "pwm0".

### Use-Case-8

Force a FIFO underrun on channel #2 while data transmission is in progress.

The rate is set to about 12 MHz so the signal can be watched with an inexpensive logic analyzer. The data range is set to 2: only two bits per word are transmitted. This makes the peripheral read the FIFO six million times per second. The process that fills the FIFO cannot keep-up.

Four words are enqueued for transmission. If you don't see any gap, you may want to increase the number of words.

```
$ ./rpio cm set pwm -f 0 -i 21 -s 6
$ ./rpio cm switch pwm on
$ ./rpio pwm range 2 2
$ ./rpio pwm control clear
$ ./rpio gpio mode 13 0
$./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0        2
$ ./rpio pwm enqueue 0x80000000 0x80000000 0x80000000 0x80000000
berr=0 empt=1 full=0 gap1=0 gap2=1 rerr=0 sta1=0 sta2=0 werr=0
```

Observe: The GAP2 flag is raised. When watching the signal with a logic analyzer you will probably see at least one gap immediately after the first word.

You can re-iterate the test:
```
$ ./rpio pwm clear gap2
$ ./rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0        2
$ ./rpio pwm enqueue 0x80000000 0x80000000 0x80000000 0x80000000
berr=0 empt=1 full=0 gap1=0 gap2=1 rerr=0 sta1=0 sta2=0 werr=0
```

If the range is increased to 4, the process that fills the FIFO does keep-up. The GAP2 flag is not raised:
```
$ ./rpio pwm range 2 4
$ ./rpio pwm clear gap2
$ ./rpio pwm status
$ ./rpio pwm enqueue -u 0x80000000 0x80000000 0x80000000 0x80000000
berr=0 empt=0 full=0 gap1=0 gap2=0 rerr=0 sta1=0 sta2=1 werr=0
```

## Errata

This includes the errata on [eLinux](http://elinux.org/BCM2835_datasheet_errata).

Page | Description
--- | ---
138 | "read data from a FIFO storage block, which can store up to eight 32-bit words."
| | The FIFO holds 16 32-bit words. So, if only one channel is used, all 16 words make up a "block".
138 | "Both modes clocked by clk_pwm which is nominally 100MHz"
| | The "nominal" clock seems to be zero. It needs to be set-up by the clock-manager.
140 | "PWM DMA is mapped to DMA channel 5."
| | The peripheral mapping to pace DMA writes to the PWM FIFO is 5 (TI.PERMAP).
141 | "PWM clock source and frequency is controlled in CPRMAN."
| | It doesn't say what *CPRMAN* is or how to set it. Luckily there are people who dug into the topic a bit deeper. It is assumed that *CPRMAN* is the abbrevation for *Clock Power Reset MANager*; which isn't much help either. However, the people contributing to eLinux provided a description for the [clock-manager](http://elinux.org/BCM2835_registers#CM) peripheral (CM) which holds, besides others, also two registers for the PWM clock. 
141 | The base-address for the register-block is missing.
| | The base-address is 0x7e20:c000.
143 | CLRF1 is marked as RO (read-only).
| | It is write-only and reads as zero.
143 | For SBIT: "Defines the state of the output when no transmission takes place"
| | This is only true:
| | If MODE=0 & MSEN=1 & SBIT=1.
| | If MODE=1 & RANGE>32 for the 33rd "bit" and all following.
144 | "RERR1 bit sets to high when a read when empty error occurs."
| | There is no explanation under which circumstances this may happen.
145 | For EMPT1,FULL1: they are marked as RW (read-write)
| | Since a write-operation has no effect, it should be RO (read-only).
