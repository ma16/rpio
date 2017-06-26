// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Rpi_Mbox_Interface_h_
#define _Rpi_Mbox_Interface_h_

//---------------------------------------------------------------------
// Mailbox Interface
// --native interface based on peripheral access (Queue)
// --Linux interface based on /dev/vcio (Vcio)
//---------------------------------------------------------------------

#include "Message.h"
#include "Queue.h"
#include "Vcio.h"
#include "../Bus/Coherency.h"

namespace Rpi { namespace Mbox { struct Interface
{
  using shared_ptr = std::shared_ptr<Interface> ;
      
  virtual std::vector<uint32_t> query(Message const&,uint32_t *length) = 0 ;

  virtual ~Interface() {}

  static Interface::shared_ptr make(Queue,Bus::Coherency co) ;

  static Interface::shared_ptr make(Vcio::shared_ptr) ;

} ; } }

#endif // _Rpi_Mbox_Interface_h_
