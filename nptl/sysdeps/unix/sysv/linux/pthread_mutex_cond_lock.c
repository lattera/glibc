#include <pthreadP.h>

#define LLL_MUTEX_LOCK(mutex) lll_mutex_cond_lock(mutex)
#define __pthread_mutex_lock __pthread_mutex_cond_lock
#define NO_INCR

#include <nptl/pthread_mutex_lock.c>
