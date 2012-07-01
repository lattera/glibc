/*
extern pid_t __waitpid_nocancel (pid_t, int *, int) attribute_hidden;
*/
#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

/* Call __NR_wait4, providing fourth argument (struct rusage *) as NULL. */
#define PSEUDO_EXTRA move r3, zero;
#include <sysdep-cancel.h>

PSEUDO (__waitpid, wait4, 3)
ret
PSEUDO_END(__waitpid)

libc_hidden_def (__waitpid)
weak_alias (__waitpid, waitpid)
libc_hidden_weak (waitpid)
weak_alias (__waitpid, __libc_waitpid)
libc_hidden_weak (__libc_waitpid)

#endif
