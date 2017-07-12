#include "Spi0.h"

std::vector<uint8_t> RpiExt::Spi0::xfer(std::vector<uint8_t> const &idata)
{
    std::vector<uint8_t> odata ;
    odata.reserve(idata.size()) ;
    uint32_t c = 0 ;
    c |= Rpi::Spi0::ClearRx ;
    c |= Rpi::Spi0::ClearTx ;
    c |= Rpi::Spi0::Ta  ;
    this->spi.setControl(c) ;
    for (auto p=idata.begin() ; p!=idata.end() ; )
    {
	auto c = this->spi.getControl() ;
	if (c & Rpi::Spi0::Rxd) odata.push_back(this->spi.read()) ;
	if (c & Rpi::Spi0::Txd) this->spi.write(*p++) ;
	// [todo] check if there are timeout (fifo underrun)
    }
    while (odata.size() < idata.size())
    {
	auto c = this->spi.getControl() ;
	if (c & Rpi::Spi0::Rxd)
	{
	    odata.push_back(this->spi.read()) ;
	}
    }
    c = this->spi.getControl() ;
    c &= ~Rpi::Spi0::Ta ;
    this->spi.setControl(c) ;
    return odata ;
}

