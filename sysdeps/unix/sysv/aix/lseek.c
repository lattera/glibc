/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

off_t
__lseek (int fd, off_t offset, int whence)
{
  return lseek (fd, offset, whence);
}
strong_alias (__lseek, __libc_lseek)
libc_hidden_def (__lseek)
