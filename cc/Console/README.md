# Console Program

## Synopsis

```
$ ./rpio help
arguments: [BASE] MODE [help]

BASE : --base ADDRESS  # use 0x20000000 (ARMv6) or 0x3f000000
     | --devtree       # use info in /proc/device-tree/soc/ranges

If BASE is not given then the peripheral address is derived from
the processor's model name (i.e. ARMv6/7/8) in /proc/cpuinfo.

MODE : bang        # generic bit-banging
     | clock       # r/w clock sources
     | cp          # clock-pulse generator (GPIO Clock)
     | dma         # DMA setup and tests
     | device      # control a certain device
     | gpio        # r/w GPIO
     | intr        # enable/disable ARM and GPU interrupts
     | mbox        # Mailbox access
     | poke        # r/w any word in peripheral address space
     | pwm         # PWM setup
     | sample      # sample data
     | shm         # shared memory control (POSIX IPC)
     | spi0        # serial peripheral interface (controller #0)
     | spi1        # serial peripheral interface (controller #1)
     | spi-slave   # serial peripheral interface (slave)
     | throughput  # i/o and memory performance tests

Use the keyword help for additional information.
```
