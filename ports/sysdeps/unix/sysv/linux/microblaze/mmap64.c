/* Microblaze kernel has PAGE_SHIFT set to 12.
   Determine the shift dynamically with getpagesize.  */
#define MMAP2_PAGE_SHIFT -1
#include <sysdeps/unix/sysv/linux/mmap64.c>
