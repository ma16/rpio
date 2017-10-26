# Peripheral DMA

## Synopsis

```
$ ./rpio peripheral dma help
arguments: MODE

MODE : enable INDEX [off]  # enable/disable DMA channel
     | start INDEX CB CS   # start DMA transfer
     | reset INDEX         # end transmission
     | status [-d|-v]      # show status of all channels
     | ti TI+              # create TI word from flags

DMA channels:
INDEX : 0..15              # one of the 16 DMA channels

Control and Status Register CS:
   | --cs-priority 0..15               # 16:19
   | --cs-panic-priority 0..15         # 20:23
   | --cs-wait-for-outstanding-writes  # 28   
   | --cs-disdebug                     # 29   

Transfer Information TI:
   | --ti-inten               #  0   
   | --ti-tdmode              #  1   
   | --ti-wait-resp           #  3   
   | --ti-dest-inc            #  4   
   | --ti-dest-width          #  5   
   | --ti-dest-dreq           #  6   
   | --ti-dest-ignore         #  7   
   | --ti-src-inc             #  8   
   | --ti-src-width           #  9   
   | --ti-src-dreq            # 10   
   | --ti-src-ignore          # 11   
   | --ti-burst-length 0..15  # 12:15
   | --ti-permap 0..31        # 16:20
   | --ti-waits 0..31         # 21:25
   | --ti-no-wide-bursts      # 26   
```
