// BSD 2-Clause License, see github.com/ma16/rpio

#include "Error.h"

using namespace Protocol::OneWire::Bang ;

static std::string toStr(Error::Type type,int line)
{
    static char const *v[] =
      { "NotPresent","Retry","Reset","Timing","Vanished" } ;
    auto s = v[Error::TypeN(type).n()] ;
    std::ostringstream os ;
    os << "Protocol::OneWire::Bang: " << s << " #" << line ;
    return os.str() ;
}

Error::Error(Type type,int line)
    : Neat::Error(toStr(type,line)),type_(type)
{
    // void
}
