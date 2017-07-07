# Serial Peripheral Interface (SPI0)

## Synopsis

```
$ ./rpio spi0 help
arguments: MODE

MODE | control [help] # set control status word
     | dma     [help] # invoke DMA based transfer
     | div UINT16     # set clock pulse divider
     | dlen LEN       # set DLEN register
     | run            # read bytes (ASCII) from stdin and send them
     | status         # display status
```

