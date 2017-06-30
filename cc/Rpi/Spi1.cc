// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi1.h"
#include <Neat/U32.h>

uint32_t Rpi::Spi1::xfer(uint32_t tx)
{
    // reset
    auto c0 = this->getControl0() ;
    c0 |=  Flush ;
    c0 &= ~Enable ;
    this->setControl0(c0) ;

    // enable serializer
    c0 &= ~Flush ;
    c0 |= Enable ;
    this->setControl0(c0) ;

    auto status = this->status() ;
    if (0 != (status & Busy))
	throw std::runtime_error("Spi1:controller failed (busy after reset)") ;
    if (0 == (status & RxEmpty))
	throw std::runtime_error("Spi1:controller failed (rx-data after reset)") ;
    
    constexpr auto maxLen = 3u << Neat::U32::AlignRight<TxSize>() ;
    // ...even though there are 4 slots!
    if ((status & TxSize) >= maxLen)
	throw std::runtime_error("Spi1:controller failed (tx-full after reset)") ;
    this->writeEnd(tx) ;

    status = this->status() ;
    while (0 != (status & RxEmpty))
	status = this->status() ;
    auto rx = this->read() ;
    
    // disable serializer
    status = this->status() ;
    while (0 != (status & Busy))
	// for some reason it keeps busy for a "while"
	status = this->status() ;
    if (0 == (status & RxEmpty))
	throw std::runtime_error("Spi1:controller failed (rx-data after dialog)") ;
    c0 &= ~Enable ;
    this->setControl0(c0) ;

    return rx ;
}

void Rpi::Spi1::xfer(std::deque<uint32_t> const &tx,std::vector<uint32_t> *rx,bool continuous)
{
    // reset
    auto c0 = this->getControl0() ;
    c0 |=  Flush ;
    c0 &= ~Enable ;
    this->setControl0(c0) ;

    // enable serializer
    c0 &= ~Flush ;
    c0 |= Enable ;
    this->setControl0(c0) ;

    // send and receive words
    for (auto p=tx.begin() ; p!=tx.end() ; )
    {
	constexpr auto maxLen = 3u << Neat::U32::AlignRight<TxSize>() ;
	// ...even though there are 4 slots!
	auto status = this->status() ;
	if (0 == (status & RxEmpty))
	{
	    rx->push_back(this->read()) ;
	    if (rx->size() > tx.size())
	    {
		throw std::runtime_error("Spi1:controller failed (rx > tx)") ;
	    }
	}
	else if ((status & TxSize) < maxLen)
	{
	    if (continuous && (p!=tx.end())) this->writeNext(*p++) ;
	    else 	                     this->writeEnd (*p++) ;
	}
	// else: tx-fifo full and no new data arrived 
    }

    // receive remaining words
    while (rx->size() < tx.size())
    {
	auto s = this->status() ;
	if (0 == (s & RxEmpty))
	{
	    rx->push_back(this->read()) ;
	}
	else if (0 == (s & Busy))
	{
	    throw std::runtime_error("Spi1:controller failed (rx < tx)") ;
	}
	// else: serializer busy and no new data
    }

    // disable serializer
    c0 &= ~Enable ;
    this->setControl0(c0) ;
}

