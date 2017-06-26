// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Vcio_h_
#define _Rpi_Mbox_Vcio_h_

// if file does not exist than create it with: "sudo mknod /dev/vcio c 100 0"
// see
// github.com/raspberrypi/linux/blob/rpi-3.6.y/arch/arm/mach-bcm2708/include/mach/vcio.h
// github.com/raspberrypi/userland/blob/master/host_applications/linux/apps/hello_pi/hello_fft/mailbox.c
// note: system freezes if ARM base interrupt #1 (mailbox) is disabled

#include <Posix/Fd.h>
#include <vector>
#include <sys/ioctl.h> // _IOWR

namespace Rpi { namespace Mbox { struct Vcio
{
  using shared_ptr = std::shared_ptr<Vcio> ;
  
  Vcio() : fd(Posix::Fd::open("/dev/vcio",Posix::Fd::Open::RW)) {}

  void query(uint32_t *message) { fd->ioctl(_IOWR(100,0,char*),message) ; }

private: 

  Posix::Fd::shared_ptr fd ;
  
} ; } } 

#endif // _Rpi_Mbox_Vcio_h_
