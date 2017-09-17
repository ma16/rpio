# 1-Wire Bus-System

This description is based on the DS18S20 [datasheet](http://datasheets.maximintegrated.com/en/ds/DS18S20.pdf).

Please refer also to the specification that comes with your [1-Wire®](https://www.maximintegrated.com/en/products/digital/one-wire.html) device.

## Introduction

The 1-Wire Bus-system uses a single wire (i.e. the “Bus”) for communication between a “Master“ and one or more “Slave“ devices. Each device connects to the Bus via an open-drain or 3-state port. The Bus requires an external pull-up resistor of approximately 5kΩ; thus, the idle-state for the Bus is “High“. Data is transmitted over the Bus by sending a “Pulse“, that is by pulling down the voltage to “Low“ for a certain duration.

## Parasite Power Mode

The Slave device can be powered through the Vdd pin by an external supply, or it can operate in “parasite power” mode.

In parasite power mode, the Vdd pin of the device is pulled to ground. This allows to connect the device with only two wires (Bus & ground). However, not all of the Slave's operations may be supported in parasite power mode. Besides, an external switching of the Bus may be required to power the device through the Bus (by-passing the external pull-up resistor, e.g. by a MOSFET) for short periods of time.

For this document it is assumed, Vdd is connected to an external power supply (3-wire-mode).

## Number of Slaves

There can be one or multiple Slaves attached to the Bus:
* the system is referred to as a “single-drop” system if there is only one Slave;
* the system is referred to as a “multidrop” system if there are multiple Slaves.

If there are multiple Slaves attached to the Bus, the Slaves are unaware of each other. Hence, several Slaves may answer concurrently at the same time and pull the Bus Low. Sometimes such concurrent responses are desirable, in other situations however, the Master needs to single out a Slave.

The 1-Wire protocol employs a so-called Transaction Sequence. The sequence starts with a Reset Pulse (see below) and lasts until the next Reset Pulse. ROM commands (see also below) can be used to deactivate Slaves. Deactive Slaves won't respond for the remainder of the current transaction sequence. They become available again with the next Reset Pulse.

## Bus Signals

Three primitives are used for communication:
* Initialization,
* Read a single bit,
* Write a single bit.

All communication is initiated by a Pulse from the Master. The Slave has to wait for such a Pulse before it can answer.

### Initialization

The Master starts this dialog with a 480µs “Reset“ Pulse. The Slave waits 15μs to 60μs and then issues a “Presence“ Pulse of 60μs to 240μs.

```
          reset-frame      presence-frame
        <-------------> <------------------>
	     480µs              480µs
    ...+               +--------------------+...
       |               |                    .
TX     +---------------+                    .
       .               .                    .
    ...+-----------------------+       +----+...
       .               .       |       |    
RX     .               .       +-------+
        <-------------> <-----> <-----> 
          reset-pulse    pres.   pres.  
                         idle    pulse  
```

All the Master learns in this initialization sequence is
* either there is no Slave attached to the Bus if there is no Presence Pulse
* or there are one or more Slaves attached to the Bus (at least one).

### Read Time-Slot

One bit of data is transmitted within this 60µs to 120µs “Time-Slot“.

The Master starts the Time-Slot with a 1µs to 15µs Pulse. When the Slave recognizes the falling edge it either immediately issues a 15µs to 60µs Pulse on its own (0-Bit) or doesn't respond at all (1-Bit).

```
            Time-Slot
        <--------------->
            rc       
            >-<  
    ---+      +----------+ 
       |     /.   .      .
TX     +----+ .   .      .
       .init. .   .      .
       .    . .   .      .
    ---+    . .   .    +--+
       |    . .   .    |
RX(0)  +---------------+
        <-------->     >--<
            rdv        rec
```

Output data from the Slave is only valid for *rdv*=15μs within the Time-Slot. Therefore, the Master should keep the start Pulse as short as possible (*init*=1µs). The Master may also consider a capacitive delay (*rc*=1µs) that effects the time before the Bus returns to High. That leaves a small effective window (*rdv-rc-init*) to scan the bus for the Slave's response. The actual sample should be taken near the end of the *rdv*=15μs period.

Each Time-Slot period lasts at least 60μs. The minimum (idle) recovery Bus time between the raising edge within a Time-Slot and the next Time-Slot is *rec*=1μs.

### Write Time-Slot

One bit of data is transmitted within this 60µs to 120µs “Time-Slot“.

The Master sends either a 60µs to 120µs Pulse (0-Bit) or a 1µs to 15µs Pulse (1-Bit). The Slave does never respond.

```
            Time-Slot
        <--------------->
    ---+              +--+ 
       |              |
TX(0)  +--------------+ 
        <------------->--<
            write_0    rec

    ---+    +-----------+ 
       |    |
TX(1)  +----+
       write_1
```

Each Time-Slot period lasts at least 60μs. The minimum (idle) recovery Bus time between the raising edge within a Time-Slot and the next Time-Slot is *rec*=1μs.

### Cues for the Slave to Respond

The Master issues always Write Pulses immediately after the initialization sequence. These Pulses form a ROM command (see below) followed by optional Function commands. Depending on these commands, the Slave knows when to expect a Read-Time-Slot.

## Transaction Sequence

A “Transaction“ lasts from one Reset Pulse to the next Reset Pulse. It always starts with an Initialization followed by eight Write Pulses that form a ROM command. Add-on communication depends on the ROM command.

If for any reason a Transaction needs to be suspended, the Bus must be left in the idle state if the transaction is to resume. Infinite recovery time can occur between Time-Slots so long as the Bus is idle during the recovery period.

## ROM Commands (Overview)

After the Master has detected a Presence Pulse, it issues an 8-bit ROM command. The command can operate on the unique 64-bit ROM-code (address) of a Slave or it can address all attached Slaves. There is a dedicated ROM command that allows the Master to determine the address of each attached Slave. Another ROM command allows the Master to determine the address of each attached Slave with a pending Alarm condition.

| #  | Command            | Description
---: | :------            | :----------
33   | Read ROM           | Query the Slave’s ROM-code.
55   | Match ROM          | Address a single Slave by its ROM-code and issue a Function.
CC   | Skip ROM           | Address all Slaves simultaneously and issue a Function.
EC   | Alarm (ROM) Search | Same as Search ROM. However, only those Slaves with a pending Alarm do respond.
F0   | Search ROM         | Traverse the 64-bit ROM-code address-space of all attached Slaves.

All data are transmitted least significant bit first.

A Function can be issued after a Match- and after a Skip-ROM command. The actual Function type is defined by the Slave's implementation. For example, a temperature sensor may provide two Functions: start sampling and read temperature.

## ROM Code

If there are multiple Slaves on the Bus then individual Slaves can be addressed by their unique 64-bit ROM code:

Offset | Bits | Description
-----: | ---: | :----------
 0     | 8    | Family Code
 8     | 48   | Unique Serial Number
 56    | 8    | Cyclic Redundancy Check (CRC) 

A [CRC](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)-8 is provided as part of the ROM code. The polynomial function is: x^8 + x^5 + x^4 + 1 (0x31).

## Read ROM

This command can be used when there is only one Slave on the Bus. It allows the Master to read the Slave’s 64-bit ROM code without using the Search ROM procedure. If this command is used when there is more than one Slave, a data collision will occur since all Slaves attempt to respond at the same time.

## Search ROM

After a system is powered up, the Master may want to identify all the Slaves attached to the Bus. A series of Search ROM commands allows the Master to determine the number of Slaves and their ROM codes. The [1-Wire Search Algorithm](https://www.maximintegrated.com/en/app-notes/index.mvp/id/187) is described on Maxim Integrated's Application Note 187.

## Alarm Search

This command is similar to the Search ROM command. However, not all Slaves will respond; only those with a pending Alarm condition. 

## Match ROM

This command is used to address exactly one Slave by its 64-bit ROM code. The ROM command is followed by an individual Function (which depends on the type of the Slave device). Only the Slave that exactly matches the ROM code may respond to the Function. All other Slaves must ignore the Bus until the next Reset Pulse.

## Skip ROM

This command is used to address all Slaves on the Bus simultaneously. For example, the Master can make all Slaves perform a temperature conversions by issuing a Skip ROM command followed by a Start Conversion Function. The list of actually available Functions depends on the type of the Slave device.

If only one Slave is attached to the Bus, the Skip ROM command can be used instead of the Match ROM command in order to simplify the implementation or to save bandwidth.

If more than one Slave is attached and if the subsequent Function requests any kind of a data from the Slaves, a data collision will occur since all Slaves respond simultaneously at the same time.
