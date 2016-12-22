#include <pthreadP.h>

#define LLL_MUTEX_LOCK(mutex) \
  lll_cond_lock ((mutex)->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex))

/* Not actually elided so far. Needed? */
#define LLL_MUTEX_LOCK_ELISION(mutex)  \
  ({ lll_cond_lock ((mutex)->__data.__lock, PTHREAD_MUTEX_PSHARED (mutex)); 0; })

#define LLL_MUTEX_TRYLOCK(mutex) \
  lll_cond_trylock ((mutex)->__data.__lock)
#define LLL_MUTEX_TRYLOCK_ELISION(mutex) LLL_MUTEX_TRYLOCK(mutex)

/* We need to assume that there are other threads blocked on the futex.
   See __pthread_mutex_lock_full for further details.  */
#define LLL_ROBUST_MUTEX_LOCK_MODIFIER FUTEX_WAITERS
#define __pthread_mutex_lock internal_function __pthread_mutex_cond_lock
#define __pthread_mutex_lock_full __pthread_mutex_cond_lock_full
#define NO_INCR

#include <nptl/pthread_mutex_lock.c>
