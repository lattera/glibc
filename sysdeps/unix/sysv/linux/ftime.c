/* Linux defines the ftime system call but doesn't actually implement
   it.  Use the BSD implementation.  */
#include <sysdeps/unix/bsd/ftime.c>
