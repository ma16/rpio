// BSD 2-Clause License, see github.com/ma16/rpio

#include "VcMem.h"
#include <Rpi/GpuMem.h>

Rpi::Bus::Memory::Factory::shared_ptr RpiExt::VcMem::defaultFactory() 
{
    auto vcio = Rpi::Mbox::Vcio::shared_ptr(new Rpi::Mbox::Vcio()) ;
    auto iface = Rpi::Mbox::Interface::make(vcio) ;
    return Rpi::Bus::Memory::Factory::shared_ptr(
	new Rpi::GpuMem::Factory(iface,0x1000,Rpi::Mbox::Property::Memory::Mode::deserialize(0x4))) ;
}
