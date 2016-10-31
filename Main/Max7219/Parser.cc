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

#include "Parser.h"
#include <cstring> // index

void Main::Max7219::Parser::throw_error(std::string const &s)
{
  std::ostringstream os ;
  os << s << " before (" << this->row << ',' << this->col << ')' ;
  throw Error(os.str()) ;
}

int Main::Max7219::Parser::next_char()
{
  int c = this->is->get() ;
  if (c == std::char_traits<char>::eof())
    return c ;
  if (!this->is->good()) 
    throw_error("read error") ;
  if (c == '\n') {
    ++this->row ;
    this->col = 0 ;
  }
  ++this->col ;
  return c ;
}

Main::Max7219::Parser::Command::Ptr Main::Max7219::Parser::parse()
{
  while (true) {
    int c = this->next_char() ;
    while (isspace(c)) 
      c = this->next_char() ;
  
    if (c == std::char_traits<char>::eof()) 
      return Command::Ptr(new Eof) ;

    if (c == '>') return this->parse_shift() ;
    if (c == '+') return this->parse_delay() ;
    if (c == '!') return Command::Ptr(new Latch) ;
    if (c == '"') return this->parse_echo() ;
  
    if (c == '#') parse_comment() ;
    else throw_error("invalid token <"+c+'>') ;
  }
}

std::string Main::Max7219::Parser::scan(char const *set)
{
  int c = this->next_char() ;
  while (isspace(c)) 
    c = this->next_char() ;
  if (c == std::char_traits<char>::eof())
    throw_error("premature end-of-file") ;
  std::ostringstream os ; os << static_cast<char>(c) ;
  while (true) {
    if (nullptr == index(set,this->is->peek()))
      break ;
    os << static_cast<char>(this->next_char()) ;
  }
  return os.str() ;
}

void Main::Max7219::Parser::parse_comment()
{
  std::ostringstream os ; 
  while (true) {
    int c = this->next_char() ;
    if (c == std::char_traits<char>::eof())
      return ;
    if (c == '\n')
      return ;
  }
}

Main::Max7219::Parser::Command::Ptr Main::Max7219::Parser::parse_delay()
{
  auto s = scan(".0123456789E+-") ;
  auto seconds = Ui::strto<double>(s) ;
  if (seconds < 0.0)
    throw_error("delay cannot be negative") ;
  return Command::Ptr(new Delay(seconds)) ;
}

Main::Max7219::Parser::Command::Ptr Main::Max7219::Parser::parse_echo()
{
  std::ostringstream os ; 
  while (true) {
    int c = this->next_char() ;
    if (c == std::char_traits<char>::eof())
      throw_error("premature end-of-file") ;
    if (c == '"')
      break ;
    os << static_cast<char>(c) ;
  }
  return Command::Ptr(new Echo(os.str())) ;
}

Main::Max7219::Parser::Command::Ptr Main::Max7219::Parser::parse_shift()
{
  auto s = scan("xXoO0123456789ABCDEFabcdef") ;
  auto data = Ui::strto<uint16_t>(s) ;
  return Command::Ptr(new Shift(data)) ;
}
