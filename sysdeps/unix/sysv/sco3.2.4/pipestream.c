/* SCO 3.2v4 does have `waitpid'.
   Avoid unix/pipestream.c, which says we don't.  */
#include <sysdeps/posix/pipestream.c>
