# Serial Peripheral Interface (SPI-Slave)

## Synopsis

```
$ ./rpio spi-slave help
arguments: MODE [help]

MODE | clear-tx  # clear tx-fifo
     | control   # set control register
     | read      # read octet from rx-fifo
     | read-all  # read all octets from rx-fifo
     | read-tx   # read octet from tx-fifo (test-mode)
     | reset     # reset status register
     | status    # display registers
     | write     # write word to tx-fifo
     | write-rx  # write word to rx-fifo (test-mode)
```
