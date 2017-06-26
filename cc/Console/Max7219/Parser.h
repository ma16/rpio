// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Console_Max7219_Parser_h_
#define _Console_Max7219_Parser_h_

#include <Neat/Error.h>
#include <Ui/strto.h>
#include <deque>
#include <memory> // shared_ptr

namespace Console { namespace Max7219 { struct Parser {

  struct Error : Neat::Error
  {
    Error(std::string const &s) : Neat::Error("Console::Max7219::Parser:"+s) {}
  } ;

  struct Command { using shared_ptr = std::shared_ptr<Command> ; virtual ~Command() {} } ;

  struct Delay : public Command { double   seconds ; Delay(double   seconds) : seconds(seconds) {} } ;
  struct Echo  : public Command { std::string text ; Echo (std::string text) : text      (text) {} } ;
  struct Eof   : public Command {} ;
  struct Latch : public Command {} ;
  struct Shift : public Command { uint16_t    data ; Shift (uint16_t   data) : data      (data) {} } ;

  Command::shared_ptr parse() ;

  Parser(std::istream *is) : is(is) {}
  
private:

  std::istream *is ; unsigned row=1 ; unsigned col=0 ;

  void throw_error(std::string const &s) ;

  int next_char() ;
  
  std::string scan(char const *set) ;

  void                parse_comment() ;
  Command::shared_ptr parse_delay() ;
  Command::shared_ptr parse_echo() ;
  Command::shared_ptr parse_shift() ;
  
} ; /* Parser */ } /* Max7219 */ } /* Console */

// [future] use boost::variant instead of Command inheritance. currently,
// just including <boost/variant.hpp> results in lots of warnings though.
// (in libboost1.55-dev:armhf)

#endif // _Console_Max7219_Parser_h_
    
