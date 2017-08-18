# 1-Wire Bus-System

The description here is based on the DS18S20's [datasheet](http://datasheets.maximintegrated.com/en/ds/DS18S20.pdf). Please refer to specification that comes with your [1-Wire®](https://www.maximintegrated.com/en/products/digital/one-wire.html) device.

## Abstract

The 1-Wire bus-system uses a single data wire for communication between a Master and one or more Slave devices. Each device interfaces to the data-line via an open drain or 3-state port. The bus requires an external pullup resistor of approximately 5kΩ; thus, the idle state for the bus is high.

A Slave is identified by its unique 64-bit ROM Code:

Offset | Bits | Description
-----: | ---: | :----------
 0 | 8 | Family Code
 8 | 48 | Unique Serial Number
 56 | 8 | Cyclic Redundancy Check (CRC) 

The lower-layer protocol is based on four Signals:

Signal | Source | Description
:----- | :------| :----------
Reset Pulse | Master | All components on the bus are reset.
Presence Pulse | Slave | All Slaves indicate their presence (simultaneously) after a Reset.
Write Time-Slot | Master | Transmit a single bit from Master to Slave.
Read Time-Slot |  Slave | Transmit a single bit from Slave to Master.

The communication is initiated by the Master with a Reset Pulse, followed by the Slave(s)' Presence Pulse(s). Then, the Master issues a (ROM) Command to address one or several Slaves, followed by an optional Function call.

Command | Description
:------ | :----------
Search ROM | Identify the ROM Codes of all Slaves
Read ROM | If there is only one Slave: read the Slave’s 64-bit ROM Code 
Match ROM | Address a specific Slave by its ROM Code
Skip ROM | Address all Slaves simultaneously
Alarm ROM Search | Identify the ROM Codes of all Slaves that hold a pending Alarm

The Function that can be issued after addressing a Slave is defined by the Slave's implementation. For example, a temperature sensor may provide two Functions: start sampling and read temperature.

## Hardware Configuration

There can be one or multiple Slaves:
* the system is referred to as a “single-drop” system if there is only one Slave;
* the system is referred to as a “multidrop” system if there are multiple slaves.

The Slave can be powered by an external supply on the VDD pin, or it can operate in “parasite power” mode, which allows the Slave to be connected on only two pins (instead of three). In parasite power mode external switching is required for the data line and not all Slave's operations may be supported. For the remainder of document, it is assumed, the device is connected to an external power supply.

All data are transmitted least significant bit first.

## Signalling

All Signals are initiated by the Master. That is, the Slave needs to wait for a condition provided by the Master before the Slave can pull the data-line.

If for any reason a transaction needs to be suspended, the bus MUST be left in the idle state if the
transaction is to resume. Infinite recovery time can occur between bits so long as the 1-Wire bus is in the inactive (high) state during the recovery period.

### Initialization Sequence

All communication begins with an initialization sequence that consists of a Reset Pulse from the Master followed by simultaneous Presence Pulses from all Slaves.

* The Master transmits (Tx) the Reset Pulse by pulling the bus low for a minimum of 480μs.
* The Master then releases the bus and goes into receive mode (Rx).
* When the bus is released, the 5kΩ pullup resistor pulls the 1-Wire bus high.
* When the Slave detects this rising edge, it waits 15μs to 60μs and then transmits a Presence Pulse by pulling the bus low for 60μs to 240μs.

When the Slave sends the Presence Pulse in response to the Reset Pulse, it is indicating to the Master that it is on the bus and ready to operate. All the Master learns is whether there is either no Slave or at least one.

### Read/Write Time Slots

The Master writes data to the DS18S20 during write time slots and reads data from the DS18S20 during read-time slots. One bit of data is transmitted over the 1-Wire bus per time slot.

### Write Tome Slot

There are two types of write time slots: “Write 1” time slots and “Write 0” time slots. The Master uses a Write 1 time slot to write a logic 1 to the DS18S20 and a Write 0 time slot to write a logic 0 to the DS18S20.

All write time slots must be a minimum of 60μs in duration with a minimum of a 1μs recovery time between individual write slots. Both types of write time slots are initiated by the Master pulling the 1-Wire bus low.

To generate a Write 1 time slot, after pulling the 1-Wire bus low, the Master must release the 1-Wire bus within 15μs. When the bus is released, the 5kΩ pullup resistor will pull the bus high.

To generate a Write 0 time slot, after pulling the 1-Wire bus low, the Master must continue to hold the bus low for the duration of the time slot (at least 60μs).

The slave samples the 1-Wire bus during a window that lasts from 15μs to 60μs after the Master initiates the write time slot. If the bus is high during the sampling window, a 1 is written to the slave. If the line is low, a 0 is written to the slave.

### Read-Time Slot

The slave can only transmit data to the Master when the Master issues read-time slots. Therefore, the Master must generate read-time slots immediately after issuing any Read command, so that the slave can provide the requested data.

All read-time slots must be a minimum of 60μs in duration with a minimum of a 1μs recovery time between slots. A read-time slot is initiated by the Master device pulling the 1-Wire bus low for a minimum of 1μs and then releasing the bus).

After the Master initiates the read-time slot, the slave will begin transmitting a 1 or 0 on bus. The DS18S20 transmits a 1 by leaving the bus high and transmits a 0 by pulling the bus low.

When transmitting a 0, the DS18S20 will release the bus by the end of the time slot, and the bus will be pulled back to its high idle state by the pullup resister.

Output data from the slave is valid for 15μs after the falling edge that initiated the read-time slot. Therefore, the Master must release the bus and then sample the bus state within 15μs from the start of the slot.

T-INIT (>= 1us) : Master holds V-PU = Low
T-RC            : Duration after Master releases V-PU=Low and until V-PU reaches V-IH
T-SAMPLE        : Master scans V-PU

The sum of T-INIT, T-RC, and T-SAMPLE must be less than 15μs for a read-time slot. The system timing margin is maximized by keeping T-INIT and T-RC as short as possible and by locating the Master sample time during read-time slots towards the end of the 15μs period.

## Transaction Sequence

Each dialogue between Master and Slave(s) is started by a Reset Signal.

The transaction sequence for accessing a slave device is as follows:

* Initialization
* ROM Command (followed by any required data exchange)
* Function Command (followed by any required data exchange)

Exceptions to this sequence are the Search ROM [F0h] and Alarm Search [ECh] commands.

### Initialization

All transactions on the 1-Wire bus begin with an initialization sequence. The initialization sequence consists of a reset pulse transmitted by the Master followed by presence pulse(s) transmitted by the slave(s). The presence pulse lets the Master know that slave devices are on the bus and are ready to operate.

### ROM Commands

After the Master has detected a presence pulse, it can issue a ROM command. These commands operate on the unique 64-bit ROM codes of each slave device and allow the Master to single out a specific device if many are present on the 1-Wire bus. These commands also allow the Master to determine how many and what types of devices are present on the bus or if any device has experienced an alarm condition. There are five ROM commands, and each command is 8 bits long. The Master device must issue an appropriate ROM command before issuing a DS18S20 function command.

| # | Type
---: | :---
33 | Read ROM
55 | Match ROM
CC | Skip ROM
EC | Alarm Search
F0 | Search ROM

#### Read ROM

This command can be used when there is only one slave on the bus. It allows the Master to read the slave’s 64-bit ROM code without using the Search ROM procedure. If this command is used when there is more than one slave present on the bus, a data collision will occur when all the slaves attempt to respond at the same time.

#### Search Rom

When a system is initially powered up, the Master must identify the ROM codes of all slave devices on the bus, which allows the Master to determine the number of slaves and their device types. The Master learns the ROM codes through a process of elimination that requires the Master to perform a Search ROM cycle (i.e., Search ROM command followed by data exchange) as many times as necessary to identify all of the slave devices. After every Search ROM cycle, the Master must return to the Initialization in the transaction sequence.

#### Alarm Search

The operation of this command is identical to the operation of the Search ROM command except that only slaves with a set alarm flag will respond. This command allows the Master device to determine if any DS18S20s experienced an alarm condition during the most recent temperature conversion. After every Alarm Search cycle (i.e., Alarm Search command followed by data exchange), the Master must return to the Initialization in the transaction sequence.

#### Match ROM

The match ROM command followed by a 64-bit ROM code sequence allows the Master to address a specific slave device on a multidrop or single-drop bus. Only the slave that exactly matches the 64-bit ROM code sequence will respond to the function command issued by the Master: all other slaves on the bus will wait for a reset pulse.

#### Skip ROM

The Master can use this command to address all devices on the bus simultaneously without sending out any ROM code information. For example, the Master can make all DS18S20s on the bus perform simultaneous temperature conversions by issuing a Skip ROM command followed by a Convert command.

If this ROM command is used when there is more than one slave present on the bus, and if the subsequent Function command requests any kind of a data from the Slave, a data collision will occur since the slaves attempt to respond at the same time.

## CRC Generation

CRC bytes are provided as part of the DS18S20’s 64-bit ROM code and in the 9th byte of the scratchpad memory. The scratchpad CRC is calculated from the data stored in the scratchpad, and therefore it changes when the data in the scratchpad changes. To verify that data has been read correctly, the Master must re-calculate the CRC from the received data.

The equivalent polynomial function of the CRC is:

CRC = X8 + X5 + X4 + 1

### Calculation by Shift Register

This circuit consists of a shift register and XOR gates. The shift register bits are initialized to 0.

(see illustration)

Starting with the least significant data bit, one bit at a time should be shifted into the shift register. After shifting in the 56th bit from the ROM or the most significant bit of byte 7 from the scratchpadall data bits, the polynomial generator will contain the calculated CRC.

Next, the received 8-bit CRC must be shifted into the circuit. At this point, if the re-calculated CRC was correct, the shift register will contain all 0s.



