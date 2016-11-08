# rpio -- Raspberry Pi Input Output

release.3 (Unstable)

```
arguments: [BASE] MODE [help]

BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000
     | --devtree       # use info in /proc/device-tree/soc/ranges

If BASE is not given then the peripheral address is derived from
the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.

MODE : clock       # r/w clock sources
     | gpio        # r/w GPIO
     | max7219     # dot-matrix control for the MAX7219 circuit
     | mcp3008     # ADC control for the MCP3008 circuit
     | poke        # r/w any word in peripheral address space
     | throughput  # i/o and memory performance tests

Use the keyword help for additional information.
```

## clock
```
arguments: MODE [help]

MODE : count   # verify clock resolution
     | set     # configure ARM counter
     | status  # display clock values

The supported clock sources are:
* arm    : ARM Free Running Counter (B00:420+4)
* bcm    : BCM System Timer Peripheral Counter (003:004+8)
* chrono : C++ chrono::steady_clock (clock_gettime(CLOCK_MONOTONIC))

The clock resolutions are (seconds):
* arm    : up to 2.5E-9
* bcm    : 1E-6
* chrono : 1E-9
```

## gpio
```
arguments: MODE [help]

MODE : mode    # set mode
     | output  # set level
     | pull    # switch resistor
     | status  # display status

Arguments may require a PINS value:

PINS : NO             # a single pin number
     | -l NO[,NO]...  # a set of pin numbers as list
     | -m MASK        # a set of pin number as bit mask
     | all            # all pins
```

## max7219
```
arguments: LOAD CLK DIN MODE

LOAD : Pi's pin number connected to MAX7219 LOAD pin
 CLK : Pi's pin number connected to MAX7219  CLK pin
 DIN : Pi's pin number connected to MAX7219  DIN pin

MODE : -d DATA+ | -i | -f FILE [-r N]

 -d : shift DATA+ and latch
 -f : read COMMAND+ until eof from FILE and execute thereafter
 -i : read COMMAND from standard-input and execute (repeat until eof)
 -r : execute N times (default 1)

COMMAND+ : COMMAND | COMMAND COMMAND+
   DATA+ :    DATA |    DATA    DATA+

 COMMAND : > DATA     # shift
         | + SECONDS  # delay
         | !          # latch (i.e. load)
         | " TEXT     # echo on standard-output

    DATA : 0..65535
 SECONDS : floating point decimal number
    TEXT : string terminated by double quotes (")
```

## mcp3008
```
arguments: CS CLK DIN DOUT [-m] MODE [help]

  CS: Pi's pin to feed MCP's CS pin
 CLK: Pi's pin to feed MCP's CLK pin
 DIN: Pi's pin to feed MCP's DIN pin
DOUT: Pi's pin to read MCP's DOUT pin

-m: enable monitoring to detect communication errors

MODE :   rate  # perform throughput test
     | sample  # read one or more samples
```

## poke
```
rguments: MODE

MODE : get ADDR
     | set ADDR WORD

ADDR : peripheral address offset: 0..FFFFFC
WORD : 32-bit integer number

For example, the GPIO input levels are located at offset 0x200034.
```

## throughput
```
arguments : REP MODE

     MODE : 0:1        DST1 ( blck M | iter | pool M )
          | 0:n N      DSTN ( blck M | iter | libc   )
          | 1:0   SRC1      ( blck M | iter | pool M )
          | 1:1   SRC1 DST1 ( blck M | iter | pool M )
          | 1:n N SRC1 DSTN ( blck M | iter |        )
          | n:0 N SRCN      ( blck M | iter |        )
          | n:1 N SRCN DST1 ( blck M | iter |        )
          | n:n N SRCN DSTN ( blck M | iter | libc   )

      REP : number of repetitions
        N : number of 32-bit words to transfer
   blck M : use buffer of M 32-bit words
   pool M : perform M copy/read/write operations at once
     libc : Lib-C's memset (0:n) or memcpy (n:n)

SRC1,DST1 : LOCATION1
SRCN,DSTN : LOCATIONN

LOCATION1 : plain                 # use virtual memory
          | port PAGE-NO PAGE-IX  # use peripheral address

LOCATIONN : plain                 # use virtual memory

  PAGE-NO : peripheral page number as offset 0..FFF (e.g. 0x200 for GPIO)
  PAGE-IX : offset within page 0..FFC (e.g. 0x34 for GPIO input levels)
```
