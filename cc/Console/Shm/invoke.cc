// BSD 2-Clause License, see github.com/ma16/rpio

// --------------------------------------------------------------------
// Shared memory control
// --------------------------------------------------------------------

#include "../rpio.h"
#include <Linux/PhysMem.h>
#include <Posix/base.h> // page_size
#include <Posix/shm.h>
#include <Ui/strto.h>
#include <iostream>

namespace Console { namespace Shm {

static void createInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto key = Ui::strto<key_t>(argL->pop()) ;
  auto nbytes = Ui::strto<size_t>(argL->pop()) ;
  auto ugo = Ui::strto(argL->pop(),Posix::shm::ugo_t()) ;
  argL->finalize() ;
  auto id = Posix::shm::create(key,nbytes,ugo) ;
  std::cout << id.as_unsigned() << std::endl ;
}
    
static void idInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto key = Ui::strto<key_t>(argL->pop()) ;
  auto nbytes = Ui::strto<size_t>(argL->pop()) ;
  argL->finalize() ;
  auto id = Posix::shm::get(key,nbytes) ;
  std::cout << id.as_unsigned() << std::endl ;
}
    
static void destroyInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto id = Ui::strto(argL->pop(),Posix::shm::id_t()) ;
  argL->finalize() ;
  Posix::shm::destroy(id) ;
}
    
static void sizeInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto id = Ui::strto(argL->pop(),Posix::shm::id_t()) ;
  argL->finalize() ;
  auto stat = Posix::shm::stat(id) ;
  std::cout << stat.shm_segsz << std::endl ;
}
    
static void layoutInvoke(Rpi::Peripheral*,Ui::ArgL *argL)
{
  auto id = Ui::strto(argL->pop(),Posix::shm::id_t()) ;
  argL->finalize() ;
  auto p = Posix::shm::attach(id) ;
  auto nbytes = Posix::shm::stat(id).shm_segsz ;
  auto phys = Linux::PhysMem::map(p,nbytes) ;
  for (decltype(nbytes) i=0 ; i<nbytes ; i+=Posix::page_size())
    std::cout << std::hex << phys->phys(i).first << '\n' ;
}
    
void invoke(Rpi::Peripheral *rpi,Ui::ArgL *argL)
{
  if (argL->empty() || argL->peek() == "help") { 
    std::cout << "arguments: MODE\n"
	      << '\n'
	      << "MODE : create  KEY SIZE UGO\n"
      	      << "     | destroy ID\n"
      	      << "     | id      KEY SIZE\n"
      	      << "     | layout  ID\n"
      	      << "     | size    ID\n"
	      << '\n'
	      << "ID   : a unique integer (returned by create and id)\n"
	      << "KEY  : a unique IPC key as integer value\n"
	      << "SIZE : size in bytes\n"
	      << "UGO  : access permission as integer (0777)\n"
	      << std::flush ;
    return ;
  }
  std::string arg = argL->pop() ;
  if      (arg ==   "create")  createInvoke(rpi,argL) ;
  else if (arg ==  "destroy") destroyInvoke(rpi,argL) ;
  else if (arg ==       "id")      idInvoke(rpi,argL) ;
  else if (arg ==   "layout")  layoutInvoke(rpi,argL) ;
  else if (arg ==     "size")    sizeInvoke(rpi,argL) ;
  else throw std::runtime_error("not supported option:<"+arg+'>') ;
}

} /* Shm */ } /* Console */
