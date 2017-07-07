# Read/Write memory locations directly

## Synopsis

```
$ ./rpio poke help
arguments: [-p] ADDR MODE

MODE :   get
     |  read NBYTES FILE
     |   set WORD
     | write FILE 

ADDR = virtual memory address
-p   = ADDR is relative to peripheral base address
WORD = 32-bit integer number

for example: <-p get 0x200034> displays GPIO levels
```
