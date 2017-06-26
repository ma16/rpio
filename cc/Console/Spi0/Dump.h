// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Dump_h
#define Console_Spi0_Dump_h

// --------------------------------------------------------------------
// todo:
// the destructor has to deal with thread termination on exceptions
// --------------------------------------------------------------------
    
#include "BoundedQueue.h"
#include <ostream>
#include <thread>
#include <vector>

namespace Console { namespace Spi0 {

struct Dump
{
    using unique_ptr = std::unique_ptr<Dump> ;
    
    using Buffer = std::vector<uint8_t> ;

    static unique_ptr make(std::ostream *os,size_t max) ;
    
    void schedule(Buffer &&buffer)
    {
	if (!buffer.empty())
	    q.put(std::move(buffer)) ;
	// note that empty buffers are used as EOF marker
    }

    size_t pending() const
    {
	return q.size() ;
    }
    
    void stop()
    {
	q.put(Buffer()) ;
	t.join() ;
    }

private:

    Dump(std::ostream *os,size_t lo,size_t hi) : os(os),q(lo,hi) {}
    
    void run() ;
    
    void write(Buffer const &buffer) ;

    std::ostream *os ;

    BoundedQueue<Buffer> q ;
    
    std::thread t ;
} ;

} }
    
#endif // Console_Spi0_Dump_h
