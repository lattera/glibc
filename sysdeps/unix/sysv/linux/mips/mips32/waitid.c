#include <libc-internal.h>

/* MIPS forces a frame pointer for five-argument syscalls using
   alloca, so resulting in "inlining failed in call to 'do_waitid':
   function not inlinable".  */
DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Winline");
#include <sysdeps/unix/sysv/linux/waitid.c>
