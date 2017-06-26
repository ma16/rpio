// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Throughput_Buffer_h_
#define _Console_Throughput_Buffer_h_

#include <Rpi/Peripheral.h>
#include <Ui/ArgL.h>

namespace Console
{
  namespace Throughput
  {
    struct Buffer
    {
      using shared_ptr = std::shared_ptr<Buffer> ;
      virtual uint32_t* front() = 0 ;
      virtual size_t size() const = 0 ;
      virtual ~Buffer() {}
    } ;

    Buffer::shared_ptr locate(Rpi::Peripheral *rpi,size_t nwords,Ui::ArgL *argL) ;
  }
}
  
#endif // _Console_Throughput_Buffer_h_
