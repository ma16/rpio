// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Neat_stream_h_
#define _Neat_stream_h_

#include "NotSigned.h"
#include <fstream>

namespace Neat
{
  void open(std::fstream *io,
            std::string const &fname,
            std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) ;

  void open(std::ifstream *is,
            std::string const &fname,
            std::ios_base::openmode mode = std::ios_base::in) ;

  void open(std::ofstream *os,
            std::string const &fname,
            std::ios_base::openmode mode = std::ios_base::out) ;

  using ustreamoff = Neat::NotSigned<std::streamoff> ;
  
  ustreamoff tellg(std::istream *is) ;

  void seekg(std::istream *is,std::streamoff,std::ios_base::seekdir way) ;

  ustreamoff size(std::istream *is) ;

  using ustreamsize = Neat::NotSigned<std::streamsize> ;
  
  void read(std::istream *is,void *buffer,ustreamsize size) ;

  void write(std::ostream *os,void const *buffer,ustreamsize size) ;
}

#endif // _Neat_stream_h_
