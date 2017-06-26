// BSD 2-Clause License, see github.com/ma16/rpio

#include "base.h"
#include "Error.h"
#include "shm.h"
#include <Neat/cast.h>
#include <cassert>
#include <sstream>

Posix::shm::id_t Posix::shm::create(key_t key,size_t nbytes,ugo_t permissions)
{
  auto shmflg = IPC_CREAT | IPC_EXCL | permissions.value() ;
  auto result = shmget(key,nbytes,static_cast<int>(shmflg)) ;
  // ...[future] introduce conversion that makes sure all set bits are included
  if (result >= 0)
    return id_t::make(Neat::as_unsigned(result)) ;
  assert(result == -1) ;
  std::ostringstream os ;
  os << "shm:create(" << key << ',' << nbytes << ",0" << std::oct << permissions.value() << "):" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}

Posix::shm::id_t Posix::shm::get(key_t key,size_t nbytes)
{
  auto result = shmget(key,nbytes,0) ;
  if (result >= 0)
    return id_t::make(Neat::as_unsigned(result)) ;
  assert(result == -1) ;
  std::ostringstream os ;
  os << "shm:get(" << key << ',' << nbytes << "):" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}

shmid_ds Posix::shm::stat(id_t id)
{
  shmid_ds buffer ;
  auto result = shmctl(id.as_signed(),IPC_STAT,&buffer) ;
  if (result == 0)
    return buffer ;
  assert(result == -1) ;
  std::ostringstream os ;
  os << "shm:stat(" << id.as_signed() << "):" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}

void Posix::shm::destroy(id_t id)
{
  auto result = shmctl(id.as_signed(),IPC_RMID,nullptr) ;
  if (result == 0)
    return ;
  assert(result == -1) ;
  std::ostringstream os ;
  os << "shm:destroy(" << id.as_signed() << "):" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}

void* Posix::shm::attach(id_t id)
{
  auto result = shmat(id.as_signed(),nullptr,0) ;
  if (result != (void*)-1)
    return result ;
  std::ostringstream os ;
  os << "shm:attach(" << id.as_signed() << "):" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}

void Posix::shm::detach(void const *addr)
{
  auto result = shmdt(addr) ;
  if (result == 0)
    return ;
  assert(result == -1) ;
  std::ostringstream os ;
  os << "shm:detach():" << Posix::strerror(errno) ;
  throw Error(os.str()) ;
}
