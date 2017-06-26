// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef Posix_Signal_h
#define Posix_Signal_h

namespace Posix { namespace Signal {

    void   block(int signal) ;
    bool pending(int signal) ;

} }

#endif // Posix_Signal_h
