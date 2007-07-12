#include_next <kernel-features.h>

/* NPTL can always assume all clone thread flags work.  */
#ifndef __ASSUME_CLONE_THREAD_FLAGS
# define __ASSUME_CLONE_THREAD_FLAGS	1
#endif
