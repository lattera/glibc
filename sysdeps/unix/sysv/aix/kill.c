/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

int
__kill (pid_t pid, int sig)
{
  return kill (pid, sig);
}
