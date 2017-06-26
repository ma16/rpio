// BSD 2-Clause License, see github.com/ma16/rpio

#include "Spi0.h"

uint16_t Rpi::Spi0::getDivider() const
{
    return static_cast<uint16_t>(this->page->at<0x8/4>()) ;
}

void Rpi::Spi0::setDivider(uint16_t i)
{
    this->page->at<0x8/4>() = i ;
}

void Rpi::Spi0::transceive(size_t nbytes,uint8_t const tx[],uint8_t rx[])
{
    // reset
    auto ctrl = this->getControl() ;
    ctrl |= ClearRx ; 
    ctrl |= ClearTx ;
    ctrl &= ~Ta ;
    this->setControl(ctrl) ;

    // control word for rx-fifo
    ctrl &= ~ClearRx ; 
    ctrl &= ~ClearTx ;
    ctrl |= Ta  ;
    auto dlen = this->getDlen() ;
    this->write(static_cast<uint8_t>(ctrl),dlen) ;

    // index to tx and rx buffers
    decltype(nbytes) ti=0,ri=0 ;

    // put data into tx-fifo
    while (true)
    {
	if (ti == nbytes)
	    break ;
	if (0 == (this->getControl() & Txd))
	    break ;
	this->write(tx[ti++]) ;
    }

    // start transfer
    this->setControl(ctrl) ;
    // --CS pin is pulled to low
    // --MOSI pin is pulled to low
    // --if tx-fifo is non-empty, the first word is used to set up the
    //   CS- and the DLEN-register
    // --tx-fifo is clocked-out (MOSI); rx-fifo is clocked-in (MISO);
    //   with SCLK providing the beat
    // --when the tx-fifo runs empty and the transfer of the last bit
    //   is completed then the CS.DONE flag is set;  the flag will be
    //   cleared automatically if new data is placed in the tx-fifo
    // --the transfer stops when the rx-fifo gets completely filled up;
    //   once there is space again (due to a client's read) the
    //   transfer is continued automatically

    // receive data and send remaining data (busy loop)
    while (ri < nbytes)
    {
	auto ctrl = this->getControl() ;
	if (0 != (ctrl & Rxd))
	    rx[ri++] = this->read() ;
	if (ti == nbytes)
	    continue ;
	if (0 != (ctrl & Txd))
	    this->write(tx[ti++]) ;
	// [note] depending on the transfer speed, it may be useful to
	// goto sleep when the tx-fifo is full and the rx-fifo is empty
	// (to prevent heavy cpu load)
    }    

    while (0 == (this->getControl() & Done))
	;
    // ...should actually become true w/o any delay

    // deassert CS- and MOSI-pin
    this->setControl(ctrl & ~Ta) ;
}
