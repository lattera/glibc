/* SCO has a bug where `waitpid' will never return if SIGCHLD is blocked.
   They have acknowledged that this is a bug but I have not seen nor heard
   of any forthcoming fix.  */

#define WAITPID_CANNOT_BLOCK_SIGCHLD

/* SCO 3.2v4 does have `waitpid'.
   Avoid unix/system.c, which says we don't.  */

#include <sysdeps/posix/system.c>
