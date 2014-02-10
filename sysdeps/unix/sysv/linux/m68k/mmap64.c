/* ColdFire and Sun 3 kernels have PAGE_SHIFT set to 13 and expect
   mmap2 offset to be provided in 8K pages.  Determine the shift
   dynamically with getpagesize.  */
#define MMAP2_PAGE_SHIFT -1
#include <sysdeps/unix/sysv/linux/mmap64.c>
