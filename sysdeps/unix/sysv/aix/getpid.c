/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

int
__getpid (void)
{
  return getpid ();
}
libc_hidden_def (__getpid)
