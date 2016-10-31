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

#ifndef _Main_Max7219_Parser_h_
#define _Main_Max7219_Parser_h_

#include <Neat/Error.h>
#include <Ui/strto.h>
#include <deque>
#include <memory> // unique_ptr

namespace Main { namespace Max7219 { struct Parser {

  struct Error : Neat::Error
  {
    Error(std::string const &s) : Neat::Error("Main::Max7219::Parser:"+s) {}
  } ;

  struct Command { using Ptr = std::unique_ptr<Command> ; virtual ~Command() {} } ;

  struct Delay : public Command { double   seconds ; Delay(double   seconds) : seconds(seconds) {} } ;
  struct Echo  : public Command { std::string text ; Echo (std::string text) : text      (text) {} } ;
  struct Eof   : public Command {} ;
  struct Latch : public Command {} ;
  struct Shift : public Command { uint16_t    data ; Shift (uint16_t   data) : data      (data) {} } ;

  Command::Ptr parse() ;

  Parser(std::istream *is) : is(is) {}
  
private:

  std::istream *is ; unsigned row=1 ; unsigned col=0 ;

  void throw_error(std::string const &s) ;

  int next_char() ;
  
  std::string scan(char const *set) ;

  void         parse_comment() ;
  Command::Ptr parse_delay() ;
  Command::Ptr parse_echo() ;
  Command::Ptr parse_shift() ;
  
} ; /* Parser */ } /* Max7219 */ } /* Main */

// [future] use boost::variant instead of Command inheritance. currently,
// just including <boost/variant.hpp> results in lots of warnings though.
// (in libboost1.55-dev:armhf)

#endif // _Main_Max7219_Parser_h_
    
