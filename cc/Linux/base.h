// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Linux_base_h_
#define _Linux_base_h_

#include <cstdint> // uint32_t

namespace Linux 
{
  
  uint32_t phys_page(uint64_t vpno) ;
  // ...[todo] argument: info; return uint64_t

} /* Linux */

#endif // _Linux_base_h_
