/* SVR4 uses the POSIX dup2.  */
#include <sysdeps/posix/__dup2.c>

weak_alias (__dup2, dup2)
