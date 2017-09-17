# DS18B20

The [DS18B20](../../../Device/Ds18b20/Ds18x20.md) is a digital thermometer which acts as Slave device on a [1-Wire Bus-System](../../../Protocol/OneWire/1wire.md).

## Synopsis

```
$ rpio device ds18b20 help
arguments: [-f FREQ | -s] PIN COMMAND...

convert ADDRESS [-d] [-r] [-w]
pad     ADDRESS [-d] [-r]
power   ADDRESS [-d] [-r]
restore ADDRESS [-d] [-r] [-w]
rom             [-d] [-r]
save    ADDRESS [-d] [-r] [-w]
search  [-a]    [-d] [-r]
write   ADDRESS [-d] [-r] TH TL CF

convert: Convert-T Function
    [-w] wait for completion and then issue 'pad'

pad: Read Scratch-Pad Function

power: Read Power-Supply Function
    echo 1 if Vdd powered
         0 if in parasite power-mode

restore: Recall E2 Function
    [-w] wait for completion

rom: Read ROM command

save: Copy Scratch-Pad Function
    [-w] wait for completion

search: Alarm/Search-ROM command
    Issues a series of commands to displays all attached devices
    [-a] do Alarm-Search command instead of Seach-ROM

write: Write Scratch-Pad Function
    TH: high temperature threshold (uint8)
    TL:  low temperature threshold (uint8)
    CF: configuration register (uint8)

Options:
[-d] display debug messages
[-r] retry if timing wasn't met
[-s] omit frequency info message

ADDRESS: [-a XX:XX...]
    do Match-ROM command instead of Skip-ROM
    e.g. -a 28:ff:40:16:c2:16:03:28

FREQ: ARM Counter frequency
```

## Examples

GPIO pin # 2 is used in all examples.
```
$ rpio gpio mode 2 i
$ rpio gpio output 2 lo
```
The pin must operate in Input mode and the Output level must be preset to Low (both is the default after boot).

The timing is based on the ARM counter which needs to be enabled (it is disabled after boot). 
```
$ rpio clock set on 0
```
The prescaler of the ARM counter can vary (here 0, which provides the maximum resolution of 4ns @ a core clock of 250 MHz).

### Enumerate

Find all attached devices:
```
$ rpio device ds18b20 -s 2 search -r
28:ff:40:16:c2:16:03:28 crc:ok
28:ff:62:87:c4:16:04:6f crc:ok
28:ff:12:18:c2:16:03:2b crc:ok
28:ff:d6:46:c2:16:03:db crc:ok
```

There are four attached devices. The 8-byte ROM-code is display least-significant-byte-first. The first byte is the Family-Code. The last byte is the CRC.

### Scratch-Pad

Read the scratch-pad of a selected device:

```
$ rpio device ds18b20 -s 2 pad -a 28:ff:40:16:c2:16:03:28 -r 
50:05:55:00:7f:ff:7f:10:c2 crc:ok
```
The scratch-pad is displayed least-significant-byte-first.

* The first two bytes hold the temperature (85°C).
* The next two bytes hold the Alarm thresholds (high=85°C, low=0°C).
* The next byte holds the resolution (12 bits).

Read the scratch-pad of all attached devices:

```
$ rpio device ds18b20 -s 2 search -r |\
  while read a crc ; do echo -n "$a $crc " ;\
  rpio device ds18b20 -s 2 pad -a $a -r ; done
28:ff:40:16:c2:16:03:28 crc:ok 50:05:55:00:7f:ff:7f:10:c2 crc:ok
28:ff:62:87:c4:16:04:6f crc:ok 50:05:55:00:5f:ff:5f:10:73 crc:ok
28:ff:12:18:c2:16:03:2b crc:ok 50:05:55:00:3f:ff:3f:10:b9 crc:ok
28:ff:d6:46:c2:16:03:db crc:ok 50:05:55:00:1f:ff:1f:10:08 crc:ok
```

The four devices differ in the configuration of the resolution: The resolutions are 12, 11, 10 and 9 bits.

Note, if multiple devices are attached, the address of the device needs to be given.

Otherwise all devices will answer at the same time and the responses overlap (by a logical AND operation):
```
$ rpio device ds18b20 -s 2 pad -r 
50:05:55:00:1f:ff:1f:10:00 crc:failure
```

### Measure Temperature

A new measurement is triggered by the convert Function:

```
$ rpio device ds18b20 -s 2 convert -r
```

The command can be broadcasts to all attached devices. It takes a moment for a device to complete the measurement (up to 750ms).

The scratch-pad has to be read again to obtain the temperatures:

```
$ rpio device ds18b20 -s 2 search -r |\
  while read a crc ; do echo -n "$a $crc " ;\
  rpio device ds18b20 -s 2 pad -a $a -r ; done
28:ff:40:16:c2:16:03:28 crc:ok 65:01:55:00:7f:ff:7f:10:9c crc:ok
28:ff:62:87:c4:16:04:6f crc:ok 64:01:55:00:5f:ff:5f:10:6e crc:ok
28:ff:12:18:c2:16:03:2b crc:ok 64:01:55:00:3f:ff:3f:10:a4 crc:ok
28:ff:d6:46:c2:16:03:db crc:ok 68:01:55:00:1f:ff:1f:10:2a crc:ok
```
The temperatures are: 22.3125°C, 22.25°C, 22.25°C and 22.5°C.

### Response Time

It is also possible to trigger the temperature measurement and wait for the complettion:
```
$ rpio device ds18b20 -s 2 convert -a 28:ff:40:16:c2:16:03:28 -r -w
66:01:55:00:7f:ff:7f:10:59 crc:ok (6.28e-01s)
$ rpio device ds18b20 -s 2 convert -a 28:ff:62:87:c4:16:04:6f -r -w
64:01:55:00:5f:ff:5f:10:6e crc:ok (3.10e-01s)
$ rpio device ds18b20 -s 2 convert -a 28:ff:12:18:c2:16:03:2b -r -w
64:01:55:00:3f:ff:3f:10:a4 crc:ok (1.61e-01s)
$ rpio device ds18b20 -s 2 convert -a 28:ff:d6:46:c2:16:03:db -r -w
68:01:55:00:1f:ff:1f:10:2a crc:ok (8.22e-02s)
```
Here it takes:
* 628ms @ 12-bit resolution (spec. max. is 750ms)
* 310ms @ 11-bit (375ms)
* 161ms @ 10-bit (187.5ms)
* 82.2ms @ 9-bit (93.75ms)

### Miscelleaneous

This is a bit-banged userland implementation. It does not use Raspbian's 1-Wire [w1-gpio](https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=65137) kernel support. As with any Linux userland implementation, there is no real-time guarantee. Hence, deferred signals need to be detected and communication has be to be retried if so.

Commands are automatically retried if the -r option is given. If not given, the odds are quite high that the command aborts. For example:
```
$ rpio device ds18b20 -s 2 pad
exception caught:Protocol::OneWire::Bang: Retry #101
```

The debug option (-d) provides a bit more verbose output. For example:
```
$ rpio device ds18b20 -s 2 search -d -r
28:ff:40:16:c2:16:03:28 crc:ok 0001010011111111000000100110100001000011011010001100000000010100
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #101
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #89
Protocol::OneWire::Bang: Retry #101
28:ff:62:87:c4:16:04:6f crc:ok 0001010011111111010001101110000100100011011010000010000011110110
Protocol::OneWire::Bang: Retry #89
Protocol::OneWire::Bang: Retry #89
28:ff:12:18:c2:16:03:2b crc:ok 0001010011111111010010000001100001000011011010001100000011010100
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #101
Protocol::OneWire::Bang: Retry #131
Protocol::OneWire::Bang: Retry #131
28:ff:d6:46:c2:16:03:db crc:ok 0001010011111111011010110110001001000011011010001100000011011011
```
