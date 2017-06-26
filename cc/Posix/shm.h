// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Posix_shm_h_
#define _Posix_shm_h_

#include <sys/shm.h> // key_t
#include <Neat/Enum.h>
#include <Neat/NotSigned.h>

namespace Posix { namespace shm {

using id_t = Neat::NotSigned<int> ;

using ugo_t = Neat::Enum<unsigned,0x1ff> ;
  
id_t create(key_t,size_t,ugo_t) ;
  
id_t get(key_t,size_t) ;

id_t get(key_t,size_t) ;
 
shmid_ds stat(id_t) ;
 
void destroy(id_t) ;

void* attach(id_t) ;

void detach(void const *addr) ;
 
} /* Shm */ } /* Posix */

#endif // _Posix_shm_h_
