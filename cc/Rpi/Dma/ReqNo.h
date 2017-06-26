// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Permap_h_
#define _Rpi_Dma_Permap_h_

// --------------------------------------------------------------------
//																			       // Peripheral Mapping (§4.2.1.3, p.51, Bit:16-20 of TI)
//
// Indicates the peripheral number (1-31) whose ready signal shall be
// used to control the rate of the transfers, and whose panic signals
// will be output on the DMA AXI bus. Set to 0 for a continuous
// un-paced transfer.
//
// --------------------------------------------------------------------
//																			       // Peripheral DREQ Signals (§4.2.1.3, pp.61, re-written)
//
// A DREQ (Data Request) mechanism is used to pace the data flow
// between the DMA and a peripheral:
//
// Reading:
//
// (1) The peripheral sets DREQ if data is available to read.
//
// (2) The DMA waits until DREQ is set before launching a read
//     operation. It will then wait for the data to be returned before
//     re-checking DREQ (and if set starting the next read).
//
// (3) Once the peripheral receives a read request it should remove its
//     DREQ as soon as possible to prevent the DMA from re-sampling the
//     same DREQ assertion.
//
// Notes:
//
//    The DMA can be configured to perform single-read operations
//    or read-bursts. In case of bursts it will wait for all data to be
//    returned before re-checking the DREQ again.
//
//    DREQ’s are required when reading from APB peripherals as the AXI-
//    to-APB bridge will not wait for an APB peripheral to be ready and
//    will just perfom the APB read regardless. Thus an APB peripheral
//    needs to make sure that it has all of its read data ready before
//    it drives its DREQ high.
//  
//    DREQ’s are not required when reading from AXI peripherals. In
//    this case, the DMA will request data from the peripheral and the
//    peripheral will only send the data when it is available. The DMA
//    will not request data that is does not have room for, so no
//    pacing of the data flow is required.
//
// Writing:
//
// (1) The peripheral sets DREQ if data is available to be written to.
//
// (2) The DMA waits until DREQ is set before launching a write
//     operation.
//
// (3) Once the peripheral receives the data it should remove its
//     DREQ as soon as possible to prevent the DMA from re-sampling the
//     same DREQ assertion.
//
// Notes:
//
//     When writing to peripherals, a DREQ is always required to pace
//     the data (thus also for AXI peripherals).
//
//     Due to the pipelined nature of the AXI bus system, several
//     writes may be in flight before the peripheral receives any data
//     and withdraws its DREQ signal. Thus the peripheral must ensure
//     that it has sufficient room in its input FIFO to accommodate the
//     maximum amount of data that it might receive. If the peripheral
//     is unable to do this, the DMA WAIT_RESP mechanism can be used to
//     ensure that only one write is in flight at any one time. However
//     this is a less efficient transfer mechanism.
//

// A permanent DREQ signal is assigned to following peripherals:
//
// ...
//
// Each DMA channel can be configured to select the DREQ signal number
// to be used to pace the reading and and the signal number to be used
// to paces the writing. The DREQ line for signal number 0 is
// permanently enabled and can be used if no pacing is required.
//  
// --------------------------------------------------------------------
//																			       // The mapping of peripherals to DREQ’s is as follows:
//																			       // --------------------------------------------------------------------

namespace Rpi { namespace Dma { enum ReqNo
{
  AlwaysOn          =  0, // use if no DREQ signal is required
  Dsi               =  1, // [*1]
  PcmTx             =  2, // PCM / I2S Audio (§8)
  PcmRx             =  3, // PCM / I2S Audio (§8)
  Smi               =  4, // [*1]
  Pwm               =  5, // Pulse Width Modulator (§9)
  SpiTx             =  6, // SPI # 0 (§10)
  SpiRx             =  7, // SPI # 0 (§10)
  SlaveTx           =  8, // SPI/BSC Slave (§11) [*2]
  SlaveTx           =  9, // SPI/BSC Slave (§11) [*2]
  Unused            = 10,
  Emmc              = 11, // External Mass Media Controller (§5)
  UartTx            = 12, // UART (§12)
  SdHost            = 13, // External Mass Media Controller (§5)
  UartRx            = 14, // UART (§12)
  Dsi               = 15, // [*1]
  SlimbusMcTc       = 16, // [*1]
  Hdmi              = 17, // [*1]
  SlimbusMcRx       = 18, // [*1]
  SlimbusDc0        = 19, // [*1]
  SlimbusDc1        = 20, // [*1]
  SlimbusDc2        = 21, // [*1]
  SlimbusDc3        = 22, // [*1]
  SlimbusDc4        = 23, // [*1]
  ScalerFifo0AndSmi = 24, // [*1] [*3]
  ScalerFifo1AndSmi = 25, // [*1] [*3]
  ScalerFfio2AndSmi = 26, // [*1] [*3]
  SlimbusDc5        = 27, // [*1]
  SlimbusDc6        = 28, // [*1]
  SlimbusDc7        = 29, // [*1]
  SlimbusDc8        = 30, // [*1]
  SlimbusDc9        = 31, // [*1]

  // [*1] There is no further description in the Peripheral datasheet
  // [*2] However, it says in §11.2: "BSC and SPI controllers do not
  //      have DMA connected, hence DMA is not supported."
  // [*3] The datasheet elaborates: "The SMI element of the Scaler
  //      FIFO 0 & SMI DREQs can be disabled by setting the SMI_DISABLE
  //      bit in the DMA_DREQ_CONTROL register in the system arbiter
  //      control block."
  
} ; } }

#endif // _Rpi_Dma_Permap_h_
