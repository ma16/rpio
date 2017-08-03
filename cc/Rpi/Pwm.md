# Pulse Width Modulator (PWM)

See [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf): Pulse Width Modulator (§9). Please refer also to the errata section at the end of this page.

Highlights:
* Two output channels.
* Operates also as generic serializer.
* 16-word deep FIFO with 32-bit words.
* DMA pacing.
* Clock-manager.

## Clock Rate

Clock source and pre-scaler are configured by the [clock-manager](../Cm) peripheral (CM):
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
CLRF | Write 1 to clear the FIFO. Write 0 has no effect. Read returns always zero.
MODE# | 0 = PWM, 1 = Serial 
MSEN# | 0 = coherent, 1 = mark-space; only effective if MODE=0
POLA# | 0 = normal, 1 = inverse output polarity
PWEN# |	0 = disable, 1 = enable transmission 
RPTL# | 0 = normal, 1 = repeat last-sent word if idle; only effective if USEF=1
SBIT# | 0 = Low, 1 = High output if idle
USEF# | 0 = use Data register, 1 = use FIFO instead

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
9 | STA1 | Status | #1 | -
10 | STA2 | Status | #2 | -

A bit remains set until cleared. Write 1 to clear a bit. Write 0 has no effect.

**BERR**

An error has occurred while writing to registers via APB. This may happen if the bus tries to write successively to same set of registers faster than the synchroniser block can cope with. Multiple switching may occur and contaminate the data during synchronisation.

**RERR**

There is no explanation under which circumstances this flag ist set.

A test shows that the flag is not set when reading from an empty FIFO. Reading returns 0x70776d30 which spells "pwm0":

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

The DMA controller supports writing the FIFO. The DMA controller needs to be set-up with TI.PERMAP=5. This is the peripheral mapping to pace write operations for the PWM-FIFO.

Offset | Size | Name | Abstract | Default
-----: | ---: | :--- | :------- | ------:
0 | 8 | DREQ | Threshold when to put DREQ signal on the bus | 7
8 | 8 | PANIC | Threshold when to put PANIC signal on the bus | 7
31 | 1 | ENAB | Enable DMA pacing, i.e. DREQ and PANIC signals | -

That is: the DREQ signal is raised on the AXI bus when the number entries in the FIFO drops to DMAC.DREQ. This signal tells the DMA controller to resume the transfer (and stop if the signal is cleared).

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

The overflow won't happen if the DMA controller uses TI.WAIT_RESP=1. This makes the DMA controller wait until it receives the AXI write response for each write. It ensures that multiple writes cannot get stacked in the AXI bus  pipeline. The maximum transfer rate will drop accordingly.

## Errata

This includes the errata on [eLinux](http://elinux.org/BCM2835_datasheet_errata).

Page | Description
--- | ---
138 | "read data from a FIFO storage block, which can store up to eight 32-bit words."
| | The FIFO holds 16 32-bit words. So, if only one channel is used, all 16 words make up a "block".
138 | "Both modes clocked by clk_pwm which is nominally 100MHz"
| | The "nominal" clock seems to be zero. It needs to be set-up by the clock-manager.
141 | "PWM clock source and frequency is controlled in CPRMAN."
| | It doesn't say what *CPRMAN* is or how to set it. Luckily there are people who dug into the topic a bit deeper. It is assumed that *CPRMAN* is the abbrevation for *Clock Power Reset MANager*; which isn't much help either. However, the people contributing to eLinux provided a description for the [clock-manager](http://elinux.org/BCM2835_registers#CM) peripheral (CM) which holds, besides others, also two registers for the PWM clock. 
141 | The base-address for the register-block is missing.
| | The base-address is 0x7e20:c000.
143 | CLRF1 is marked as RO (read-only).
| | It is write-only and reads as zero.
143 | For RPTL=0: "Transmission interrupts when FIFO is empty"
| | For none-DMA mode: when the FIFO gets empty, the last word is repeated regardless whether this bit is set or not; even if the FIFO is cleared (CLRF). However, there is nothing to repeat if the serializer starts up with a cleared FIFO.
143 | For SBIT: "Defines the state of the output when no transmission takes place"
| | This is only true:
| | If MODE=0 & MSEN=1 & SBIT=1.
| | If MODE=1 & RANGE>32 for the 33rd "bit" and all following.
143 | USEF
| | [defect] channel #2 seems not always to be working properly in FIFO-mode. *Sometimes* no transmission takes place (in serializer mode) even if the FIFO is full and STA=1. The FIFO simply stays full. [open issue]
144 | "BERR sets to high when an error has occurred while writing to registers via APB. This may happen if the bus tries to write successively to same set of registers faster than the synchroniser block can cope with. Multiple switching may occur and contaminate the data during synchronisation."
| | This kind of problem can be observed when writing twice in a row to the Control register. Since the effects are unpredictable, application developers should check for BERR after each write, and abort if set. Or try to prevent BERR at all; e.g. by adding additional read-cyles.
144 | For STA: "1 means channel is transmitting data."
| | For USEF=1 & RPTL=1: if PWEN is enabled on an empty FIFO then STA is set immediately.
| | For USEF=1 & RPTL=0: if PWEN is enabled on an empty FIFO then STA remains cleared until a word is written to the FIFO (or RPTL is enabled).
| | [defect] STA may remain set even if PWEN is cleared. This can be observed sometimes for operations that cause BERR=1. In order to clear STA, BERR needs to be cleared first, thereafter PWEN.
144 | "RERR1 bit sets to high when a read when empty error occurs."
| | There is no explanation under which circumstances this may happen.
145 | For EMPT1,FULL1: they are marked as RW (read-write)
| | Since a write-operation has no effect, it should be RO (read-only).
145 | EMPT1
| | Note that the serializer may still be busy with the transfer even after the FIFO gets empty. So the flag is no indicator to disable PWEN after the end of a transmission.
