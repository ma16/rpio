// Copyright (c) 2016, "ma16". All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// This project is hosted at https://github.com/ma16/rpio

#ifndef _Posix_Fd_h_
#define _Posix_Fd_h_

#include <Neat/NotSigned.h>

#include <string>

#include <memory> // std::unique_ptr
#include <fcntl.h> // O_RDWR...
#include <sys/types.h> // off_t

namespace Posix
{
  struct Fd
  {
    using uoff_t = Neat::NotSigned<off_t> ;
    // ...off_t arguments should not be negative

    using ussize_t = Neat::NotSigned<ssize_t> ;
    // ...ssize_t arguments should not be negative

    enum class Open { RO=O_RDONLY,WO=O_WRONLY,RW=O_RDWR } ;
    
    static std::unique_ptr<Fd> open(char const path[],Open mode) ; 

    ussize_t read(void *buf,ussize_t count) ;

    ~Fd() ;

    Fd           (Fd const&) = delete ;
    Fd& operator=(Fd const&) = delete ;
    
  private:

    friend class MMap ;
    
    int i ; std::string path ;

    Fd(int i,std::string &&path) : i(i),path(std::move(path)) {}

    // [note] there are many file types as regular,device,pipe,socket,...
  } ;
} 

#endif // _Posix_Fd_h_
