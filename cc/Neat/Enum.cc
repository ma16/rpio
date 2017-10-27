// BSD 2-Clause License, see github.com/ma16/rpio

#include "Enum.h"
#include "Error.h"
#include <sstream> 

void Neat::Enum_throw(unsigned long long max,unsigned long long i)
{
    std::ostringstream os ;
    os << "Enum:" << std::to_string(i) << ' '
       << "out of range (0," << std::to_string(max) << ')' ;
    throw Error(os.str()) ;
}
