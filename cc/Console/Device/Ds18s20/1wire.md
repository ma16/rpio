# 1-Wire Bus-System

This description is based on the DS18S20's [datasheet](http://datasheets.maximintegrated.com/en/ds/DS18S20.pdf).

Please refer to the specification that comes with your [1-Wire®](https://www.maximintegrated.com/en/products/digital/one-wire.html) device.

## Abstract

The 1-Wire bus-system uses a single wire (i.e. the “bus”) for communication between a Master and one or more Slave devices. Each device connects to the bus via an open-drain or 3-state port. The bus requires an external pull-up resistor of approximately 5kΩ; thus, the idle state for the bus is High.

There may be multiple Slaves on the bus. These Slaves are identified by their unique 64-bit ROM code:

Offset | Bits | Description
-----: | ---: | :----------
 0 | 8 | Family Code
 8 | 48 | Unique Serial Number
 56 | 8 | Cyclic Redundancy Check (CRC) 

The protocol on the bus is based on four signals:

Signal | Description
:----- | :----------
Reset Pulse | The Master initiates communication.
Presence Pulse | The Slaves indicate their presence (simultaneously).
Write Time-Slot | Transmit a single bit from Master to Slave.
Read Time-Slot | Transmit a single bit from Slave to Master.

The communication is initiated by the Master with a Reset Pulse, followed by the Slave's Presence Pulse. Then, the Master issues a command to address one or several Slaves, followed by an optional function call.

Command | Description
:------ | :----------
Search ROM | Identify the ROM codes of all Slaves. This may take several calls.
Read ROM | Read the Slave’s ROM code if there is only one Slave.
Match ROM | Address a specific Slave by its ROM code and issue a function.
Skip ROM | Address all Slaves simultaneously and issue a function.
Alarm (ROM) Search | Identify the ROM codes of all Slaves that hold a pending Alarm. This may take several calls.

A function that can be issued after addressing a Slave. The actual Function type is defined by the Slave's implementation. For example, a temperature sensor may provide two Functions: start sampling and read temperature.

## Hardware Configuration

All data are transmitted least significant bit first.

There can be one or multiple Slaves:
* the system is referred to as a “single-drop” system if there is only one Slave;
* the system is referred to as a “multidrop” system if there are multiple Slaves.

The Slave can be powered by an external supply on the VDD pin, or it can operate in “parasite power” mode, which allows the Slave to be connected on only two pins (instead of three). In parasite power mode external switching is required for the bus and not all Slave's operations may be supported. For the remainder of the document it is assumed, the Slave is connected to an external power supply.

## Transaction Sequence

The transaction sequence for a Master-Slave dialog is as follows:

* Initialization Sequence
* ROM Command and optional response
* Optional Function Command and response

There are Exceptions to this sequence (see ROM Commands).

If for any reason a transaction needs to be suspended, the bus must be left in the idle state if the
transaction is to resume. Infinite recovery time can occur between bits so long as the bus is idle during the recovery period.

## Signalling

All the four bus signals are initiated by the Master. That is, the Slave needs to wait for a condition provided by the Master before the Slave can pull the bus Low.

### Initialization Sequence

All transactions on the bus begin with an initialization sequence. The initialization sequence consists of a Reset Pulse issued by the Master followed by simultaneous Presence Pulses issued by the Slaves.

* The Master issues the Reset Pulse by pulling the bus Low.
* The Master releases the bus after a minimum of 480μs. The 5kΩ pull-up resistor pulls the bus High again.
* When a Slave detects the rising edge, it waits 15μs to 60μs and then issues a Presence Pulse by pulling the bus Low.
* The Slave releases the bus after a period of 60μs to 240μs. The 5kΩ pull-up resistor pulls the bus High again.

All the Master learns in the initialization sequence is whether there is either no Slave on the bus or at least one. In order to find out how many Slaves there actually are, the Master needs to issue Search ROM commands (see below).

### Time-Slots

One bit of data is transmitted per Time-Slot.

Each Time-Slot is initiated by the Master by pulling the bus Low.

Each Time-Slot period lasts at least 60μs. The minimum recovery time between Time-Slots is 1μs.

Type | Description
:--- | :---
Write Time-Slot | The Master sends data to the Slave.
Read Time-Slot | The Master receives data from the Slave.

#### Write

Type | Description
:--- | :---
Write-1 Time-Slot | The Master pulls the bus Low for up to 15μs.
Write-0 Time-Slot | The Master pulls the bus Low for at least 60μs.

The Slave samples the bus any time during a window that lasts from 15μs to 60μs after the falling edge.

#### Read

A Read Time-Slot is initiated by the Master by pulling the bus Low for a minimum of 1μs.

Type | Description
:--- | :---
Read-1 Time-Slot | The Slave leaves the bus High.
Read-0 Time-Slot | The Slave pulls the bus Low until the end of the Time-Slot.

Output data from the Slave is valid only for the 15μs after the falling edge that initiated the Read Time-Slot. Therefore, the Master must release the bus fast and then sample the bus level within 15μs from the start of the Time-Slot.

Timing | Description
:--- | :---
T-INIT | Period the Master pulls the bus Low (at least 1μs).
T-RC | Period the bus would reach High level again (thru pull-up resistor).
T-SAMPLE | Period the Master can scan the bus for the Slave's output.

The sum of T-INIT, T-RC, and T-SAMPLE must be less than 15μs. T-INIT and T-RC should be kept as short as possible. The actual sample should be taken near the end of the 15μs window.

Note: After the initialization sequence the Master issues Write Time-Slots that form a ROM command (see below) followed by optional Function commands. Depending on the command, the Slave knows when to expect one or multiple Read Time-Slot signals.

## ROM Commands

After the Master has detected a Presence Pulse, it can issue a ROM command. These commands operate on the unique 64-bit ROM codes of each Slave and allow the Master to single out a Slave if more than one is present. The ROM commands also allow the Master to determine how many and what types of Slaves are present or if any Slave has experienced an Alarm condition.

There are five ROM commands, and each command is 8 bits long:

| # | Type
:--- | :---
33 | Read ROM
55 | Match ROM
CC | Skip ROM
EC | Alarm Search
F0 | Search ROM

### Read ROM

This command can be used when there is only one Slave on the bus. It allows the Master to read the Slave’s 64-bit ROM code without using the Search ROM procedure. If this command is used when there is more than one Slave, a data collision will occur since all Slaves attempt to respond at the same time.

### Search ROM

After a system is powered up, the Master may want to identify all the Slaves attached to the bus. A series of Search ROM commands allows the Master to determine the number of Slaves and their ROM codes. The [1-Wire Search Algorithm](https://www.maximintegrated.com/en/app-notes/index.mvp/id/187) is described on Maxim Integrated's Application Note 187.

### Alarm Search

This command is similar to the Search ROM command. However, not all Slaves will respond but only those with a pending Alarm condition. 

### Match ROM

This command is used to address exactly one Slave by its 64-bit ROM code. The ROM command is followed by an individual Function (which depends on the type of the Slave device). Only the Slave that exactly matches the ROM code may respond to the Function. All other Slaves must ignore the bus until the next Reset Pulse.

### Skip ROM

This command is used to address all Slaves on the bus simultaneously. For example, the Master can make all Slaves perform a temperature conversions by issuing a Skip ROM command followed by a Start Conversion Function. The list of actually available Functions depends on the type of the Slave device.

If there is only one Slave attached to the bus, the Skip ROM command can be used instead of the Match ROM command in order to simplify the implementation or to save bandwidth.

If there is more than one Slave and if the subsequent Function requests any kind of a data from the Slaves, a data collision will occur since the Slaves respond simultaneously at the same time.

## CRC

A [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)-8 is provided as part of the Slave's 64-bit ROM code. The polynomial function is: x^8 + x^5 + x^4 + 1 (0x31).

The mathematics of a cyclic redundancy check are closely related to those of an [LFSR](https://en.wikipedia.org/wiki/Linear_feedback_shift_register):

* The circuit consists of a shift-register and XOR gates. The XOR gates tap the Bits 0, 4 and 5.
* All shift-register bits are initialized to 0.
* Data start LSB first. Data is shiftet into the MSB of the shift-register.
* All data bits are shifted one after another into the shift-register.
* After shifting is completed, the shift-register contains the CRC.
* The CRC is shifted into the circuit (as any data before).
* After shifting, the shift-register will contain all 0s if the CRC did match the data.
