// BSD 2-Clause License, see github.com/ma16/rpio

#include "../Error.h"
#include "Queue.h"

void Rpi::Mbox::Queue::query(uint32_t i)
{
  if (this->readable())
    throw Error("Mbox::Queue:unexpected pending response") ;
  while (!this->writable())
    ;
  this->write(i) ;
  while (!this->readable())
    ;
  auto j = this->read() ;
  if (j != i)
    throw Error("Mbox::Queue:response does not match") ;
}
