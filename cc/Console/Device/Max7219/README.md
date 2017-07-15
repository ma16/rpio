# MAX7219

The MAX7219 circuit is a display driver for 7-segment numeric LED displays with up to 8 digits, bar-graph displays, or 64 individual LEDs as dot-matrices. The circuit is cascadable to control multiple devices. It operates at 4.0 to 5.5V and requires a logic-high input-voltage of at least 3.5V. Please refer to the [datasheet](https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf) for detailed information.

Note: the circuit that was used for a test (probably a knock-off) did work at an operating voltage of only 3.3V and without a level shifter.

## Abstract

The sections outlines some of the circuit's features.

### Pinout

Pin | Type | Description
:--- | :--- | :---
CLK | Serial-Clock Input | Data is read (DIN) on CLK’s rising edge and put out (DOUT) on CLK’s falling edge. The maximum clock-rate is 10 MHz. 
DIN | Serial-Data Input | Data is shifted from DIN into an internal 16-bit register on CLK’s rising edge.
DOUT | Serial-Data Output | This pin is used to daisy-chain circuits. Data is shifted out from the internal 16-bit register into DOUT on CLK's falling edge. So the DIN level becomes the DOUT level after 16.5 clock cycles.
LOAD | Load | The internal 16-bit shift-register is latched-in and processed (see Commands below) on LOAD's rising edge.

### Timing

Type | Min | Description
:--- | :--- | :---
t_ch | 50ns | CLK high-level pulse-width 
t_cl | 50ns | CLK low-level pulse-width 
t_csw | 50ns | LOAD high-level pulse-width
t_ds | 25ns | DIN setup to CLK rising-edge
t_ldck | 25ns | LOAD rising-edge to next CLK rising-edge

### Commands

The input command is 16-bit wide (MSB comes in first). The high nibble is ignored (mask:F000). The next nibble (mask:0F00) defines the register to modify. The last two nibbles (mask:00FF) may hold additional data.

Value | Register
:--- | :---
0000 | No Operation
0100 | Digit 0
0200 | Digit 1
0300 | Digit 2
0400 | Digit 3
0500 | Digit 4
0600 | Digit 5
0700 | Digit 6
0800 | Digit 7
0900 | Decode Mode
0A00 | Intensity
0B00 | Scan Limit
0C00 | Shutdown
0F00 | Display Test

On initial power-up, all registers are reset, the display is blanked, and in shutdown mode. 

#### No Operation

The last two nibbles are ignored.

This command may be useful when several circuits are daisy chained. For those circuits that do not need any change, a No-Operatio can be issued instead of overwriting the present value (with a like).

#### Set Digit N

The operation depends on the value of the Decode-Mode register.

If Decode-Mode is **activated**, the SEG:DP line (dot) is taken from bit 7 (mask:0080) and the final nibble (mask 000F) is interpreted as a 7-segment digit:

Value | Digit | Value | Digit | Value | Digit | Value | Digit 
:--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- 
0000 | '0' | 0004 | '4' | 0008 | '8' | 000C | 'H'
0001 | '1' | 0005 | '5' | 0009 | '9' | 000D | 'L'
0002 | '2' | 0006 | '6' | 000A | '-' | 000E | 'P'
0003 | '3' | 0007 | '7' | 000B | 'E' | 000F | (blank)

If Decode-Mode is **deactivated**, the segment lines are taken from the last two nibbles.

Mask | Segment
:--- | :---
0080 | DP
0040 | A
0020 | B
0010 | C
0008 | D
0004 | E
0002 | F
0001 | G

#### Decode Mode

This mode can be used to simplify the handling with 7-segment displays. The setting depends on the value of the last two nibbles (mask:00FF).

Value | Mode
:--- | :---
0000 | Deactivated
0001 | Activated for digit 0
000F | Activated for digits 0-3
00FF | Activated for digits 0-7

Note, you can see that the bits correlate with the activated digits, i.e. bit 0 corresponds to digit 0, bit 1 corresponds to digit 1, etc. The circuit that was used for a test actually supported not only the four values of the datasheet, but all possible values from 0..FF.

#### Intensity

The intensity depends on the value of the last nibble (mask:000F).

Value | Intensity (0..1)
:--- | :---
0000 | 1 / 32
0000 | 3 / 32
...  | ...
000F | 31 / 32

#### Scan-Limit

The number of active digits depends on the last three bits (mask:0007). Inactive digits stay blank.

Value | Active Digits
:--- | :---
0000 | 0
0001 | 0-1
... | ...
0007 | 0-7

The number of active digits affects also the display brightness! Let's say you have an application with 4 active digits (for instance '1000') and switch to three active digits (for instance '999'), the latter one will be brighter. Thus, the scan-limit should not be used to blank portions of the display such as leading zero suppression. 

#### Shutdown

The operation depends on the last bit (mask:0001).

Value | Operation
:--- | :---
0000 | Shutdown Mode
0001 | Normal Operation

Shutdown can be used to save power. It can also be used as an alarm to flash the display by successively entering and leaving shutdown mode.

#### Display Test

The operation depends on the last bit (mask:0001).

Value | Operation
:--- | :---
0000 | Normal Operation
0001 | Display Test Mode 

The Test mode turns on all LEDs by overriding, but not altering, all registers (including the shutdown register). All segments of all digits are activated at the maximum brightness.

## Synopsis
```
$ ./rpio device max7219
arguments: DIN LOAD CLK [-f FREQ] MODE

 DIN : Pi's pin number connected to MAX7219  DIN pin
LOAD : Pi's pin number connected to MAX7219 LOAD pin
 CLK : Pi's pin number connected to MAX7219  CLK pin

FREQ: ARM counter frequency that has been set up

MODE : -d DATA+ | -s FILE [-r N]

 -d : shift DATA+ and latch
 -r : execute N times (default 1)
 -s : read COMMAND+ until eof from FILE and execute

COMMAND+ : COMMAND | COMMAND COMMAND+
   DATA+ :    DATA |    DATA    DATA+

 COMMAND : '>' DATA     # 16-bit word to serialize
         | '+' SECONDS  # number of seconds to sleep
         | '!'          # activate LOAD pin
         | '"' TEXT '"' # print text on standard-output
```

## Example (Dot-Matrix)

In this example, a 8x8 dot-matrix is attached to the circuit.

The implementation is bit-banged. The ARM counter, which is used as clock, needs to be set up beforehand (here @ 250 MHz core-clock w/o divider):

```
$ ./rpio clock set on 0
```

We need three pins for communication with the circuit. Those pins' must operate in Output mode. We use pins 22, 23 and 24 in this example:
```
$ ./rpio gpio mode -l 22,23,24 o
```

At this point, we can run the circuit's display-test:

```
$ ./rpio device max7219 22 23 24 -d 0x0f01
$ ./rpio device max7219 22 23 24 -d 0x0f00
```

The first line should enable all segments of all digits with the highest brigthness. With the second line we return from the test mode: the display goes off again.

Note, the command line interface doesn't support names for the commands to execute. The interface only accepts the 16-bit values of the datasheet.

At this point, we assume, the circuit is still in its powered-up state: All registers of the circuit are reset. So let's switch on some lights. This time *properly*:

```
$ seq 1 1 8 | while read i ; do c=`printf "0x%02xff" $i`; \
./rpio device max7219 22 23 24 -d $c ; done
$ ./rpio device max7219 22 23 24 -d 0x0900
$ ./rpio device max7219 22 23 24 -d 0x0a00
$ ./rpio device max7219 22 23 24 -d 0x0b07
$ ./rpio device max7219 22 23 24 -d 0x0c01
```

These commands do:
* For each digit: turn on all segments
* Deactivate decode-mode
* Set lowest brightness
* Activate all digits
* Disable shutdown-mode
After execution of the third line, the display should turn from off to on.

So, what we've got here is almost the same as the test mode. However, the lights schouldn't be that bright. So let's increase the brightness step by step to its maximum:

```
$ seq 1 1 15 | while read i ; do c=`printf "0x0a0%x" $i`; \
./rpio device max7219 22 23 24 -d $c ; sleep 1 ; done
```

And now, just for fun, let's turn the display off, digit by digit:

```
$ seq 1 1 8 | while read i ; do c=`printf "0x%02x00" $i`; \
./rpio device max7219 22 23 24 -d $c ; sleep 1 ; done
```

## Example (5 Dot-Matrices)

In this example, 5 circuits, each attached to an 8x8 dot-matrix, are daisy-chained.

Setup as before:
```
$ ./rpio clock set on 0
$ ./rpio gpio mode -l 22,23,24 o
```

Run the circuit's display-test:

```
$ ./rpio device max7219 22 23 24 -d 0x0f01
$ ./rpio device max7219 22 23 24 -d 0x0000
$ ./rpio device max7219 22 23 24 -d 0x0000
$ ./rpio device max7219 22 23 24 -d 0x0000
$ ./rpio device max7219 22 23 24 -d 0x0000
```

In the first line, the display-test is activated for the first circuit. The next four lines are Nops. However, since the circuits are daisy chained, the display-test is shiftet from one to the next. All displays should be on.

The alternative is to send five commands at once and load them together. We use this to return from the display-test.

```
./rpio device max7219 22 23 24 -d 0x0f00 0x0f00 0x0f00 0x0f00 0x0f00
```

As in the example above, we initialize the registers and end the shutdown mode.

```
$ seq 1 1 8 | while read i ; do c=`printf "0x%02xff" $i`; \
./rpio device max7219 22 23 24 -d $c $c $c $c $c ; done
$ ./rpio device max7219 22 23 24 -d 0x0900 0x0900 0x0900 0x0900 0x0900
$ ./rpio device max7219 22 23 24 -d 0x0a00 0x0a00 0x0a00 0x0a00 0x0a00
$ ./rpio device max7219 22 23 24 -d 0x0b07 0x0b07 0x0b07 0x0b07 0x0b07
$ ./rpio device max7219 22 23 24 -d 0x0c01 0x0c01 0x0c01 0x0c01 0x0c01
```

All displays should be turned on with the lowest brightness.

And now, just for fun, let's turn the displays off, digit by digit:

```
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0100
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0200
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0300
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0400
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0500
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0600
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0700
$ ./rpio device max7219 22 23 24 -d 0 0 0 0 0x0800
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0100 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0200 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0300 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0400 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0500 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0600 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0700 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0 0x0800 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0100 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0200 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0300 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0400 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0500 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0600 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0700 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0 0x0800 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0100 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0200 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0300 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0400 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0500 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0600 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0700 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0 0x0800 0 0 0 
$ ./rpio device max7219 22 23 24 -d 0x0100 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0200 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0300 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0400 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0500 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0600 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0700 0 0 0 0
$ ./rpio device max7219 22 23 24 -d 0x0800 0 0 0 0
```

## Example (Scripting)

The console program supports simple scripts to control the display. The parser recognizes basically only three inputs:
* send (shift) command ('>')
* load command into circuit ('!')
* sleep a number or a fraction of seconds ('+')

There are a few Perl [scripts](../../../../script/Max7219/) to generate such files. For example:

```
$ ../../../../script/Max7219/ticker.pl 5 "The quick brown fox jumps over the lazy dog" > script
$ ./rpio device max7219 22 23 24 -s script
```

This plays a ticker with the given text rendered for 5 dot-matrices.
