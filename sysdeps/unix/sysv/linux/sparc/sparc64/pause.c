#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sysdep-cancel.h>

#define __sigprocmask(how, set, oset) \
  INLINE_SYSCALL (rt_sigprocmask, 4, how, set, oset, _NSIG / 8)

#include <sysdeps/posix/pause.c>
