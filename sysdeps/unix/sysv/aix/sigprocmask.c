/* This is a system call.  We only have to provide the wrapper.  */
#include <signal.h>

int
__sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  return sigprocmask (how, set, oset);
}
