#include <pthreadP.h>

#define LLL_MUTEX_LOCK(mutex) lll_cond_lock (mutex, /* XYZ */ LLL_SHARED)
#define LLL_MUTEX_TRYLOCK(mutex) lll_cond_trylock (mutex)
#define LLL_ROBUST_MUTEX_LOCK(mutex, id) lll_robust_cond_lock (mutex, id, /* XYZ */ LLL_SHARED)
#define __pthread_mutex_lock __pthread_mutex_cond_lock
#define NO_INCR

#include <nptl/pthread_mutex_lock.c>
