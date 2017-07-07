# Measure Memory and GPIO Throughput

## Synopsis

```
$ ./rpio throughput help
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
