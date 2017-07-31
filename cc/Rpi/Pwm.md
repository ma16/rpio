# Pulse Width Modulator (PWM)

See [BCM2835 ARM Peripherals](https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf): Pulse Width Modulator (§9). Please refer also to the errata at the end of this page.

Highlights:
* Two output channels.
* Operates as pulse-width-modulator or as generic bit-serializer.
* 16-word FIFO-size (32-bit words).
* DMA pacing.
* Clock-manager.

## Description

### PWM Signal

The PWM-signal is defined by the period and by the ratio:
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

### BSS Mode

The bits of a given 32-bit word are serialized.

### FIFO

An internal 16 x 32-bit FIFO is used to buffer incoming values. The values are serialized consecutively.

### DMA

DMA writes 32-bit words into the FIFO. A DMA signal line (DREQ=5) is provided to pace the data transfer.

### Clock Source and Frequency

The datasheet simply states:

**PWM clock source and frequency is controlled in CPRMAN.**

It doesn't say what *CPRMAN* is or how to set it. And the Raspberry Pi Foundation announced there won't be an update of the datasheet. That would make the whole PWM peripheral quite useless for application developers. 

Luckily there are people who dug into the topic a bit deeper. It is assumed that *CPRMAN* is the abbrevation for *Clock Power Reset MANager*; which isn't much help either. However, the people contributing to the eLinux provided a description for the [clock-manager](../Cm) peripheral (CM) which holds, besides others, also two registers for the PWM clock. 

## Errata

This includes the errata on [eLinux](http://elinux.org/BCM2835_datasheet_errata).

Page | Description
--- | ---
138 | "read data from a FIFO storage block, which can store up to eight 32-bit words."
| | The FIFO holds 16 32-bit words. So, if only one channel is used, all 16 words make up a "block".
138 | "Both modes clocked by clk_pwm which is nominally 100MHz"
| | The "nominal" clock seems to be zero. It needs to be set-up by the clock-manager.
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
| | There is no explanation under which circumstances this may happen. RERR1 is not set when reading from an empty FIFO. Reading the FIFO by application (CPU) simply returns "pwm0" regardless of the FIFO contents.
145 | For EMPT1,FULL1: they are marked as RW (read-write)
| | Since a write-operation has no effect, it should be RO (read-only).
145 | EMPT1
| | Note that the serializer may still be busy with the transfer even after the FIFO gets empty. So the flag is no indicator to disable PWEN after the end of a transmission.
