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

The flags FULL, EMPT, STA1 and STA2 are set and reset by the peripheral.

The flags WERR, RERR, GAP1, GAP2 and BERR are raised by the peripheral and remains set until cleared by the application. Write 1 to clear a flag. (Writing 0 has no effect.)

**BERR**

An error has occurred while writing to registers via APB. This may happen if the bus tries to write successively to the same set of registers faster than the synchronizer block can cope with. Multiple switching may occur and contaminate the data during synchronisation.

This kind of problem can be observed when writing twice in a row to the Control register (Use-Case-6). Since the effects are unpredictable, application developers should check for BERR after and try to prevent it. Adding additional read-cyles may be used as **work-around**. Still, this remains an open issue.

Note that the BERR flag appears always to be raised when the Control register is written but the clock-manager wasn't set-up yet (CM.PWM.CONTROL.ENAB=0).

**GAP**

The flag is raised the moment the underrun happens. If the flag is cleared while the channel is active (PWEN=1) and the FIFO is (still) empty, the peripheral will leave the flag unset and not raise again.

It appears the flag is often raised even tough there was no FIFO underrun (see Defects).

**GAP1**

The flag is only effective if both channels are activated (PWEN1=1 & PWEN2=1).

If only channel #1 is activated (PWEN1=1 & PWEN2=0) the peripheral acts as if RPTL1=1, and thus does never raise GAP1 (see Defects).

**RERR**

There is no explanation under which circumstances this flag ist set.

Reading the FIFO doesn't raise the flag (Use-Case-7).

**STA**

The peripheral sets the flag when data transmission is in progress and clear the flag when no data transmission takes place. Hence, when the channel is not active (PWEN=0) the flag is not set. If the channel is active (PWEN=1) and there is no data in the FIFO, the flag is cleared until new data is available (unless RPTL1=1).

**STA1**

The flag is only effective if both channels are activated (PWEN1=1 & PWEN2=1).

If only channel #1 is activated (PWEN1=1 & PWEN2=0) the peripheral acts as if RPTL1=1, and thus does never clear STA1 for idle periods (see Defects).

## FIFO Register (FIF1)

A write operation to this register appends a word onto the FIFO. The FIFO itself is 16 words deep. Each word is 32-bit wide. The FIFO can not be read. The status of the FIFO is reflected by STA.EMPT, STA.FULL, STA.RERR and STA.WERR.

When writing the register you should make sure: 
* either beforhand that the FIFO is not full
* or afterwards that no write-error has occurred.

The FIFO is shared between both channels. Hence, when both channels are enabled for FIFO usage:
* The data is shared between these channels in turn. For example, with the word sequence A B C D E F G H, the first channel will use A C E G and the second channel will use B D F H.
* The range register should hold the same value for both channels.
* The datasheet says that RPTL# is not meaningful as there is no defined channel to own the last data in the FIFO. Therefore both RPTL# flags must be set to zero. However, observations show, it doesn't has to.
* If the configuration has changed in any of the two channels, the FIFO should be cleared before writing new data.

### FIFO Underrun

A FIFO underrun in Serial mode will distort the transferred data. This might not be acceptable for many applications. Still, it cannot be prevented. At least not for data that exceeds the FIFO buffer. A FIFO underrun is obvious if the peripheral reads the FIFO faster than the application is able to write the FIFO. This may be obscured by bus overloads and process suspensions.

If the FIFO is written by DMA, an underrun is less likely. Though it is still possible at high PWM rates and small word ranges. If the FIFO is written by CPU, the writing thread can be suspended at any time for any duration.

At the very least it might be helpful to detect a FIFO underrun: This kind of information is supposed to be provided by the GAP Status flag (Use-Case-8).

The GAP flag is not raised in RPTL=1 mode. If data is transferred on channel #1 only, the GAP1 flag is never raised since channel #1 operates always as if RPTL1=1 (see Defects). The peripheral may run out of data unnoticed in the middle of a transmission.

In general, the GAP flag seems to produce many false positives, especially on a Pi model Zero (Use-Case-9). Hence, if the FIFO is written by DMA, there is no reliable indicator whether there was an underrun. This is an open issue.

If the FIFO written by CPU, the EMPT Status flag is no reliable indicator either for a FIFO underrun: Imagine that the writing thread gets suspended immediately after checking the EMPT Status flag. If the suspension lasts long enough, the FIFO runs empty. This is a less likely, but still probable scenario, especially if vast amounts of data are transferred.

The following approach may help to detect a FIFO underrun (Use-Case-9): The user data is written in blocks of 16 words (which is the FIFO size). A word is only written if there is still space (FULL=0). When 16 words are written in a row, there might have been an underrun. (It might also be a false positive.) If less then 16 words were written (when the FIFO became full again), an underrun can be ruled out.

If no peripheral is active, the FIFO should be topped-up with user data before the transfer starts. Otherwise, the FIFO should be flooded with padding words (i.e. by "fast" writes w/o checking whether there is space or not). Likewise, the end of the user data needs be padded to enable 16-word block-writes until the last word of user data was written.

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
$ rpio cm set pwm -f 0 -i 200 -s 6
$ rpio cm switch pwm on
$ rpio pwm control usef2=1 mode2=1 pwen2=1
$ rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0       20
$ rpio gpio mode 13 0
$ rpio pwm enqueue 0xf0555500
$ rpio pwm status
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
$ rpio cm set pwm -f 0 -i 200 -s 6
$ rpio cm switch pwm on
$ rpio pwm control usef1=1 mode1=1 pwen1=1
$ rpio pwm status
berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    1    0    0    0    1    1        0       20
2   0   0    0    0    0    0    0    0    0        0       20
$ rpio gpio mode 12 0
$ rpio pwm enqueue 0xf0555500
$ rpio pwm status
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
$ rpio cm set pwm -f 0 -i 200 -s 6
$ rpio cm switch pwm on
$ rpio pwm control usef2=1 mode2=1 rptl2=1 pwen2=1
$ rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   1   0    0    1    0    0    1    1    1        0       20
$ rpio gpio mode 13 0
$ rpio pwm enqueue 0xf0555500
$ rpio pwm status
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
$ rpio cm set pwm -f 0 -i 200 -s 6
$ rpio cm switch pwm on
$ rpio pwm control usef2=1 mode2=1 pwen2=1
$ rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0       20
$ rpio gpio mode 13 0
$ rpio pwm control clear
$ rpio pwm enqueue 0xf0555500
# rpio pwm status
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
$ rpio pwm clear gap2
$ rpio pwm control rptl2=1
# rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   1   0    0    1    0    0    1    1    1        0       20
$ rpio pwm control rptl2=0
# rpio pwm status
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
$ rpio pwm clear berr
$ rpio pwm berr 5
$ rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   1    0    0    0    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
1   0   0    0    0    0    0    0    0    0        0       20
$ rpio pwm clear berr
$ rpio pwm berr 20
$ rpio pwm status
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
$ rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

sta2 sta1 berr gap2 gap1 rerr werr empt full
--------------------------------------------
   0    0    0    0    0    0    0    1    0 (0x2)

# msen usef pola sbit rptl mode pwen     data    range
------------------------------------------------------
1    0    1    0    0    0    1    0        0       20
2    0    0    0    0    0    0    0        0       20
$ rpio poke -p 0x20c018 get
70776d30
```

Observe: Reading returns 0x70776d30 which spells "pwm0".

### Use-Case-8

The PWM rate is set to about 10 M/s (500/50) so the signal can still be watched with an inexpensive logic analyzer. The word's Range is set to 2: only the two MSB of each word are transmitted. This makes the PWM channel read the FIFO at a rate of 5 M/s. This is about the limit to write to the FIFO by CPU.

```
$ rpio cm set pwm -f 0 -i 50 -s 6 ;\
  rpio cm switch pwm on ;\
  rpio pwm range 2 2 ;\
  rpio pwm control usef2=1 mode2=1 pwen2=1 ;\
  rpio pwm control clear ;\
  rpio gpio mode 13 0 ;\
  rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0        2
```

Below some odds to successfully transfer small blocks of data (32 words):
```
$ seq 1 1 32 | while read ; do printf "\x00\x00\x00\x80" ; done > block_32
seq 1 1 20 | while read ; do rpio pwm clear gap2 ; rpio pwm fifo-cpu block_256 ; done
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
```

Observe: The Status is read after all words were written to FIFO: The transmission is still in progress (EMPT=0 & STA2=1). Sometimes a gap occurred (GAP2=1), sometimes not.

The same for transfer of 1k words:
```
$ seq 1 1 1000 | while read ; do printf "\x00\x00\x00\x80" ; done > block_1k
$ seq 1 1 20 | while read ; do rpio pwm clear gap2 ; chrt -f 99 rpio pwm fifo-cpu block_1k ; done
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
```

Observe: The number of transfers without a gap decreases.

The same for transfer of 50k words:
```
$ seq 1 1 50000 | while read ; do printf "\x00\x00\x00\x80" ; done > block_50k
$ seq 1 1 20 | while read ; do rpio pwm clear gap2 ; chrt -f 99 rpio pwm fifo-cpu block_50k ; done
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 1 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 0 0 1 0 0 1 0
```

Observe: This transfer lasts 10 ms (5 M/s / 50k). Since each Pi core gets suspended every 10 ms for a little time, these transfers can never succeed without a gap. The FIFO buffer is too small to span the process suspension.

### Use-Case-9

The PWM rate is set to about 10 k/s (1.92e+7/1920). The word's Range is set to 2: only the two MSB of each word are transmitted. This makes the PWM channel read the FIFO at a rate of 5 k/s. Since the FIFO can hold 16 entries, about 3 ms of process suspension can be bypassed.

```
$ rpio cm set pwm -f 0 -i 1920 -s 1 ;\
  rpio cm switch pwm on ;\
  rpio pwm range 2 2 ;\
  rpio pwm control usef2=1 mode2=1 pwen2=1 ;\
  rpio pwm control clear ;\
  rpio gpio mode 13 0 ;\
  rpio pwm status
DMA-Control: enable=0 panic=7 dreq=7

berr rerr werr empt full
------------------------
   0    0    0    1    0
# sta gap msen usef pola sbit rptl mode pwen     data    range
--------------------------------------------------------------
1   0   0    0    0    0    0    0    0    0        0       20
2   0   0    0    1    0    0    0    1    1        0        2
```

```
$ seq 1 1 1000 | while read ; do printf "\x00\x00\x00\x80" ; done > block_1k
# seq 1 1 20 | while read ; do rpio pwm clear gap2 ; rpio pwm fifo-cpu -c 0 block_1k ; done
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
underrun detected after 54 / 1000 words
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 0 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
underrun detected after 998 / 1000 words
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 0 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 1 0 0 1 0
berr,empt,full,gap1,gap2,rerr,sta1,sta2,werr: 0 0 1 0 0 0 0 1 0
```

Observe: The Status is read after all words were written to FIFO: The transmission is still in progress (EMPT=0 & STA2=1). A (potential) FIFO underrun is detected by writes in blocks of 16 words. In this example, there are two potential underruns. However, the GAP2 flags is set in many more cases.

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
