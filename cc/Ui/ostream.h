// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Ui_ostream_h_
#define _Ui_ostream_h_

#include <ostream>

inline std::ostream& operator<< (std::ostream &os,unsigned char i) { return os << static_cast<int>(i) ; }
inline std::ostream& operator<< (std::ostream &os,  signed char i) { return os << static_cast<int>(i) ; }

#endif // _Ui_ostream_h_
