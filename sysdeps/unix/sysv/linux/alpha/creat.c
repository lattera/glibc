/* sysdeps/unix/sysv/linux/wordsize-64/syscalls.list defines creat and
   creat64 for most linux targets, but on alpha creat is not a syscall.
   If we do nothing, we'll wind up with creat64 being undefined, because
   the syscalls.list assumes the creat->creat64 alias was created.  We
   could have overridden that with a create64.c, but we might as well do
   the right thing and set up creat64 as an alias.  */
#include <io/creat.c>
weak_alias(__libc_creat, creat64)
