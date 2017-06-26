// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Queue_h_
#define _Rpi_Mbox_Queue_h_

// --------------------------------------------------------------------
// Mailbox interface for communication between ARM and VideoCore
//
// Mailbox Read/Write  Peek  Sender  Status  Config
//    0      0x00      0x10   0x14    0x18    0x1c
//    1      0x20      0x30   0x34    0x38    0x3c
//
// where
//   Mailbox #0: from VC to ARM
//   Mailbox #1: from ARM to VC
// see github.com/raspberrypi/firmware/wiki/Mailboxes
//
// Ports are available at peripheral address (offset): 0x00b:880
// see github.com/raspberrypi/firmware/wiki/Accessing-mailboxes
//
// important notes:
// --several clients must not use this interface at the same time (this
//     includes the kernel)
// --this module doesn't work properly on Raspbian if ARM base
//     interrupt #1 (mailbox) is enabled (so the kernel snaps up our
//     responses)
// --the mailboxes are actually queues that can hold multiple entries
//     (the VC serves them FIFO)
// --------------------------------------------------------------------

#include "../Peripheral.h"

namespace Rpi { namespace Mbox { struct Queue
{
  Queue(Peripheral *p) : page(p->page(Peripheral::PNo::make<0x00b>())) {}
  
  using shared_ptr = std::shared_ptr<Queue> ;
  
  uint32_t read()
  {
    // invoke only if readable()
    return page->at<(0x880+0x00)/4>() ;
  }

  bool readable() const
  {
    return 0 == (page->at<(0x880+0x18)/4>() & (1u<<30)) ;
    // ...so mailbox is empty if flag is set
  }

  void write(uint32_t i)
  {
    // invoke only if writable()
    page->at<(0x880+0x20)/4>() = i ; 
  }

  bool writable() const
  {
    return 0 == (page->at<(0x880+0x38)/4>() & (1u<<31)) ;
    // ...so mailbox is full if flag is set
  }

  // send a request on #1 and wait until the response on #0 arrives
  void query(uint32_t) ;

private: std::shared_ptr<Page> page ; 

} ; } }

#endif // _Rpi_Mbox_Queue_h_
