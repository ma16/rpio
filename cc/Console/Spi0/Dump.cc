// BSD 2-Clause License, see github.com/ma16/rpio

#include "Dump.h"

Console::Spi0::Dump::unique_ptr Console::Spi0::Dump::make(std::ostream *os,size_t max)
{
    auto dump = unique_ptr(new Dump(os,0,max)) ;
    dump->t = std::thread(&Dump::run,dump.get()) ;
    return dump ;
}
    
void Console::Spi0::Dump::run()
{
    while (true)
    {
	auto &v = q.front() ;
	if (v.size() == 0)
	    break ;
	write(v) ;
	q.pop() ;
    }
    q.pop() ;
}
    
void Console::Spi0::Dump::write(Buffer const &buffer)
{
    os->write((char*)&buffer[0],(std::streamsize)(buffer.size())) ;
    if (!os->good())
	throw std::runtime_error("cannot write file") ;
}
