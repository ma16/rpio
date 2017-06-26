// BSD 2-Clause License, see github.com/ma16/rpio

#include "Parser.h"
#include <cstring> // index

void Console::Max7219::Parser::throw_error(std::string const &s)
{
  std::ostringstream os ;
  os << s << " before (" << this->row << ',' << this->col << ')' ;
  throw Error(os.str()) ;
}

int Console::Max7219::Parser::next_char()
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

Console::Max7219::Parser::Command::shared_ptr Console::Max7219::Parser::parse()
{
  while (true) {
    int c = this->next_char() ;
    while (isspace(c)) 
      c = this->next_char() ;
  
    if (c == std::char_traits<char>::eof()) 
      return Command::shared_ptr(new Eof) ;

    if (c == '>') return this->parse_shift() ;
    if (c == '+') return this->parse_delay() ;
    if (c == '!') return Command::shared_ptr(new Latch) ;
    if (c == '"') return this->parse_echo() ;
  
    if (c == '#') parse_comment() ;
    else throw_error("invalid token <"+c+'>') ;
  }
}

std::string Console::Max7219::Parser::scan(char const *set)
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

void Console::Max7219::Parser::parse_comment()
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

Console::Max7219::Parser::Command::shared_ptr Console::Max7219::Parser::parse_delay()
{
  auto s = scan(".0123456789E+-") ;
  auto seconds = Ui::strto<double>(s) ;
  if (seconds < 0.0)
    throw_error("delay cannot be negative") ;
  return Command::shared_ptr(new Delay(seconds)) ;
}

Console::Max7219::Parser::Command::shared_ptr Console::Max7219::Parser::parse_echo()
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
  return Command::shared_ptr(new Echo(os.str())) ;
}

Console::Max7219::Parser::Command::shared_ptr Console::Max7219::Parser::parse_shift()
{
  auto s = scan("xXoO0123456789ABCDEFabcdef") ;
  auto data = Ui::strto<uint16_t>(s) ;
  return Command::shared_ptr(new Shift(data)) ;
}
