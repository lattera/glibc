/* The o32 MIPS fadvise64 syscall behaves as fadvise64_64.  The ARM
   implementation of posix_fadvise works correctly for this case; the
   generic implementation mishandles it.  */
#include <sysdeps/unix/sysv/linux/arm/posix_fadvise.c>
