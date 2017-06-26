// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_h_
#define _Rpi_Dma_h_

// --------------------------------------------------------------------
// BCM2835 ARM Peripherals: chapter 4: DMA Controller
//
// "Beware that the Dma controller is direcly connected to the 
//  peripherals. Thus the Dma controller must be set-up to use 
//  the Physical (harware) addresses of the peripherals."
//
// Note that DMA invocation may harm file-system and even hardware.
// Imagine a properly set-up DMA transfer that goes rogue on early
// program termination when the DMA-assigned memory regions (which
// are still in-use until the DMA transfer is complete) are re-
// assigned to other user applications; or worse: claimed by kernel
// itself. You may encounter pink elephants. ;-)
//
// To minimize the risks, DMA-assigned memory should be persistent to
// survive process crashes. For example VideoCore-memory allocated
// thru Mailbox-interface or Linux IPC shared memory segments may serve
// this requirement.
//
// [notes]
//
// --A general description how DMA works:
//   www.embedded.com/design/mcus-processors-and-socs/4006782/Using-Direct-Memory-Access-effectively-in-media-based-embedded-applications--Part-1
//
// --DMA implementation:
//   github.com/Wallacoloo/Raspberry-Pi-Dma-Example/blob/master/dma-gpio.c
//   github.com/richardghirst/PiBits/blob/master/PiFmDma/PiFmDma.c
//   github.com/hzeller/rpi-gpio-dma-demo
//
// --Not all channels are available (used by Linux and/or VideoCore?):
//   > cat /sys/module/dma/parameters/dmachans
//   0x7F35 on Rpi2; 0111 1111 0011 0101 so 1,3,6,7,15 are in-use
//   > dmesg | grep Dma
//   allocated Dma channel 0 @ f3007000
//   (channel 2 seems to be used for SDHC)
//
// --The DMA engine is aware of L2 cache - not of the L1 cache:
//   en.wikibooks.org/wiki/Aros/Platforms/Arm_Raspberry_Pi_support#Framebuffer
//
// --DMA speed tests:
//   www.raspberrypi.org/forums/viewtopic.php?f=37&t=7696&start=50
//   www.raspberrypi.org/forums/viewtopic.php?f=33&t=116020&p
//
// [defects]
//
// --for an ARM-memory-to-memory transfer, even if ti-src-inc=0 and
//   ti-src-width=0, the DMA will copy blocks of 16 bytes (128bit)
//   which means effectively that ti-src-width is (internally) set to 1.
//   [todo] prepare test case
//
// --for a peripheral-to-ARM-memory transfer, if ti-src-width=1, the
//   DMA will still increment src by 4 and copy word by word. However,
//   not [src] is copied but [src & ~0xf] which ends up with 4
//   consecutive repetitions of the same value at the destination.
//   [todo] prepare test case
//
// https://
// "GPIO Speed using CPU and Dma on the Raspberry Pi" using mailboxes

#include "Dma/Ctrl.h"

#endif // _Rpi_Dma_h_
