// BSD 2-Clause License, see github.com/ma16/rpio

#include "ArmMem.h"
#include <Linux/Shm.h>

Rpi::Bus::Memory::shared_ptr Rpi::ArmMem::allocate(size_t nbytes,Rpi::Bus::Coherency co,bool stick)
{
  return stick
    ? Bus::Memory::shared_ptr(new ArmMem(Linux::    Shm::allocate(nbytes),co))
    : Bus::Memory::shared_ptr(new ArmMem(Linux::PhysMem::allocate(nbytes),co)) ;
}

void* Rpi::ArmMem::virt()
{
  return this->mem->virt() ;
}
  
std::pair<Rpi::Bus::Address,size_t> Rpi::ArmMem::phys(size_t ofs) 
{
  auto tuple = this->mem->phys(ofs) ;
  auto address = Bus::Address(tuple.first) ;
  address.set(this->co) ;
  return std::make_pair(address,tuple.second) ;
}
  
size_t Rpi::ArmMem::nbytes() const
{
  return this->mem->nbytes() ;
}

Rpi::ArmMem::shared_ptr Rpi::ArmMem::Factory::allocate(size_t nbytes)
{
  return ArmMem::allocate(nbytes,co,stick) ;
}
