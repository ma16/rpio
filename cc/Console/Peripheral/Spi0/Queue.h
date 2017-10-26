// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Console_Spi0_Queue_h
#define Console_Spi0_Queue_h

#include <condition_variable>
#include <deque>
#include <mutex>

namespace Console { namespace Spi0
{

template <typename T> struct Queue
{
    T const& front() const
    {
	std::unique_lock<decltype(this->m)> lock(this->m) ;
	while (this->d.empty())
	    this->c.wait(lock) ;
	return this->d.front() ;
    }

    T& front()
    {
	std::unique_lock<decltype(this->m)> lock(this->m) ;
	while (this->d.empty())
	    this->c.wait(lock) ;
	return this->d.front() ;
    }

    void pop() { d.pop_front() ; }

    void put(T &&object) 
    {
	std::unique_lock<decltype(m)> lock(this->m) ;
	this->d.push_back(std::move(object)) ;
	lock.unlock() ; 
	// ...otherwise the notified thread might immediately 
	//    block again, waiting for the lock release.
	this->c.notify_one() ;
    }

    void put(T const &object) 
    {
	std::unique_lock<decltype(m)> lock(this->m) ;
	this->d.push_back(object) ;
	lock.unlock() ; 
	// ...otherwise the notified thread might immediately 
	//    block again, waiting for the lock release.
	this->c.notify_one() ;
    }

private:

    mutable std::condition_variable c ;
    mutable std::mutex m ;
    std::deque<T> d ;
} ;

} }

#endif // Console_Spi0_Queue_h
