/* We can reuse the Linux implementation with some tricks.  */
#define __NR_sigaltstack 1
#include <sysdeps/unix/sysv/linux/sigstack.c>
