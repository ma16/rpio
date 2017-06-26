// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Dma_Cb_h_
#define _Rpi_Dma_Cb_h_

#if 0 // [TODO] to be merged with Main/Dma/Lib

namespace Rpi { namespace Dma { struct Cb // single DMA Control Block
{
  // [0] : transfer information
  // [1] : source bus-address
  // [2] : destination bus-address
  // [3] : transfer length in bytes
  //     | stride width and height (if stride mode)
  // [4] : source and destination increment (if stride mode)
  // [5] : next Cb bus-address
  // [6] : 0 ("reserved")
  // [7] : 0 ("reserved") 
    
  Ti ti ; Bus::Address src,dst ; uint32_t length,stride ; Bus::Address next ;

  Cb(Ti ti,Bus::Address src,Bus::Address dst,uint32_t length,uint32_t stride,Bus::Address next) :
    ti(ti),src(src),dst(dst),length(length),stride(stride),next(next) {}
  // [todo] length: bit 30,31 must not be set (ignored)

  std::string toStr() const ;

  static constexpr auto nbytes = sizeof(uint32_t) * 8 ; 
    
  static Cb read(uint32_t(*)[8]) ;

  void write(uint32_t const(*)[8]) const ;

  // [todo] phys next/read/write addresses must be 32-byte aligned
  
} ; } } 
#endif

#endif // _Rpi_Dma_Cb_h_
