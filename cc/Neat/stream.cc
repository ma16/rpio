// BSD 2-Clause License, see github.com/ma16/rpio

#include "stream.h"
#include "Error.h"
#include <cassert>

void Neat::open(std::fstream *io,std::string const &fname,std::ios_base::openmode mode)
{
  io->open(fname.c_str(),mode);
  if (!io->good())
    throw Error("fstream:open(" + fname + ") failed") ;
}

void Neat::open(std::ifstream *is,std::string const &fname,std::ios_base::openmode mode)
{
  is->open(fname.c_str(),mode) ;
  if (!is->good())
    throw Error("ifstream:open(" + fname + ") failed") ;
}

void Neat::open(std::ofstream *os,std::string const &fname,std::ios_base::openmode mode)
{
  os->open(fname.c_str(),mode) ;
  if (!os->good())
    throw Error("ofstream:open(" + fname + ") failed") ;
}

void Neat::read(std::istream *is,void *buffer,ustreamsize size)
{
  auto p = reinterpret_cast<char*>(buffer) ;
  is->read(p,size.as_signed()) ;
  if (is->bad())
    throw Error("istream:read():bad") ; 
  if (!is->good()) 
    throw Error("istream:read():not good") ;
  if (is->gcount() != size.as_signed())
    throw Error("istream:read():not complete") ;
}

void Neat::write(std::ostream *os,void const *buffer,ustreamsize size) 
{
  auto *p = reinterpret_cast<char const*>(buffer) ;
  os->write(p,size.as_signed()) ;
  if (os->bad()) 
    throw Error("ostream:write():bad") ; 
  if (!os->good())
    throw Error("istream:write():not good") ;
  // [note] there is no equivalent for gcount()
}

Neat::ustreamoff Neat::tellg(std::istream *is)
{
  auto pos = is->tellg() ;
  if (pos < 0) {
    assert(pos == -1) ;
    throw Error("istream:tellg():failed") ;
  }
  return ustreamoff::make(pos) ;
}

void Neat::seekg(std::istream *is,std::streamoff off,std::ios_base::seekdir way)
{
  is->seekg(off,way) ;
  if (!is->good())
    throw Error("istream:seekg():not good") ;
}

Neat::ustreamoff Neat::size(std::istream *is) 
{
  auto pos = tellg(is) ;
  seekg(is,0,std::ios::beg) ;
  auto beg = tellg(is) ;
  seekg(is,0,std::ios::end) ;
  auto end = tellg(is) ;
  seekg(is,pos.as_signed(),std::ios::beg) ;
  assert(end.as_unsigned() >= beg.as_unsigned()) ; // [future] safe_sub()
  return ustreamoff::make(end.as_unsigned() - beg.as_unsigned()) ;
}
