/* Get the stub, bypassing the "generic" NPTL code.  */
#include <posix/fork.c>
strong_alias (__fork, __libc_fork)
