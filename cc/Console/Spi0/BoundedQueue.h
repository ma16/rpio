// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_BoundedQueue_h
#define Console_Spi0_BoundedQueue_h

#include "Queue.h"
#include <cassert>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace Console { namespace Spi0
{

template <typename T> struct BoundedQueue
{
    BoundedQueue(size_t lo,size_t hi) : lo(lo),hi(hi),n(0)
    {
	assert(hi > 0) ;
	assert(lo <= hi) ;
    }

    size_t size() const
    {
	std::unique_lock<decltype(this->m)> lock(this->m) ;
	return n ;
    }
    
    T const& front() const { return q.front() ; }
    T      & front()       { return q.front() ; }

    void pop()
    {
	std::unique_lock<decltype(m)> lock(this->m) ;
	q.pop() ;
	--n ;
	if (blocked && n <= lo)
	    blocked = false ;
	lock.unlock() ; 
	// ...otherwise the notified thread might immediately 
	//    block again, waiting for the lock release.
	this->c.notify_one() ;
    }

    void put(T &&object) 
    {
	std::unique_lock<decltype(this->m)> lock(this->m) ;
	while (blocked)
	    this->c.wait(lock) ;
	q.put(std::move(object)) ;
	++n ;
	if (n >= hi)
	    blocked = true ;
    }

    void put(T const &object) 
    {
	std::unique_lock<decltype(this->m)> lock(this->m) ;
	while (blocked)
	    this->c.wait(lock) ;
	q.put(object) ;
	++n ;
	if (n >= hi)
	    blocked = true ;
    }

private:

    size_t lo,hi,n ;
    
    Queue<T> q ; 

    bool blocked = false ;

    mutable std::condition_variable c ;
    mutable std::mutex m ;
} ;

} }

#endif // Console_Spi0_BoundedQueue_h
