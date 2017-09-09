// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Posix_base_h_
#define _Posix_base_h_

#include "Error.h"
#include <string>
#include <sys/resource.h> // rusage

namespace Posix 
{
    void nanosleep(double ns) ;
  
    unsigned long page_size() ;
    // ...see sysconf(_SC_PAGESIZE)

    std::string strerror(int no) ;
    // ...see strerror_r()

    void* memalign(size_t nbytes,size_t alignment) ;
    // ...see posix_memalign()

    void   mlock(void const *p,size_t nbytes) ; 
    void munlock(void const *p,size_t nbytes) ;

    rusage getrusage() ;
} 

#endif // _Posix_base_h_
