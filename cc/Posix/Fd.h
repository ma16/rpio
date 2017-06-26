// BSD 2-Clause License, see github.com/ma16/rpio

#ifndef _Posix_Fd_h_
#define _Posix_Fd_h_

#include <Neat/NotSigned.h>

#include <string>

#include <memory> // std::shared_ptr
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

    enum class Open : int { RO=O_RDONLY,WO=O_WRONLY,RW=O_RDWR } ;

    using shared_ptr = std::shared_ptr<Fd> ;
    
    static shared_ptr open(char const path[],Open mode) ; 

    static shared_ptr create(char const path[]) ; 

    enum class Lseek : int { Begin=SEEK_SET,Current=SEEK_CUR,End=SEEK_END } ;

    uoff_t lseek(off_t ofs,Lseek mode) ;
    
    uoff_t size() ;
    // ...determined by lseek(0,End) whereby original position is recovered
    
    ussize_t read(void *buf,ussize_t count) ;
    // ...[todo] shouldn't compile if WO
    
    ussize_t write(void const *buf,ussize_t count) ;
    // ...[todo] shouldn't compile if RO
    
    void ioctl(unsigned long request,void *data) ;

    ~Fd() ;

    Fd           (Fd const&) = delete ;
    Fd& operator=(Fd const&) = delete ;
    
  private:

    friend class MMap ;
    
    int i ; std::string path ;

    Fd(int i,std::string const &path) : i(i),path(path) {}

    // [note] there are many file types as regular,device,pipe,socket,...
  } ;
} 

#endif // _Posix_Fd_h_
