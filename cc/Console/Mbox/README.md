# Peripheral Mailbox

## Synopsis

```
$ ./rpio mbox help
arguments: [-p CO] MODE

-p CO: optional peripheral Mailbox access with coherency 0..3
default: via ioctl on /dev/vcio

MODE : board      # print board information
     | clear      # clear peripheral Maxilbox queue
     | command    # print startup command line string
     | clock...   # manage peripheral clocks
     | dma        # show available DMA channels
     | device...  # manage peripheral device
     | firmware   # show firmware number
     | memory...  # manage VideoCore memory
     | ram        # show RAM information
```

