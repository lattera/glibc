#include <kernel-features.h>

#include "../../i386/xstat.c"

#ifdef __NR_stat64
# if __ASSUME_STAT64_SYSCALL == 0
/* The variable is shared between all wrappers around *stat{,64} calls.  */
int __have_no_stat64;
# endif
#endif
