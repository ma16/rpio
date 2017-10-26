# Console Program

## Synopsis

```
$ rpio help
arguments: [BASE] MODE [help]

BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000
     | --devtree       # use info in /proc/device-tree/soc/ranges

If BASE is not given then the peripheral address is derived from
the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.

MODE : defect      # defect report
     | device      # control a remote device
     | peripheral  # peripheral's access
     | poke        # r/w any word in peripheral address space
     | sample      # sample data
     | shm         # shared memory control (POSIX IPC)
     | throughput  # i/o and memory performance tests

Use the keyword help for additional information.
```
