#include <pthreadP.h>

#define LLL_MUTEX_LOCK(mutex) lll_mutex_cond_lock (mutex)
#define LLL_MUTEX_TRYLOCK(mutex) lll_mutex_cond_trylock (mutex)
#define LLL_ROBUST_MUTEX_LOCK(mutex, id) lll_robust_mutex_cond_lock (mutex, id)
#define __pthread_mutex_lock __pthread_mutex_cond_lock
#define NO_INCR

#include <nptl/pthread_mutex_lock.c>
