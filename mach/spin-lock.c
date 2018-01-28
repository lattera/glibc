#define __USE_EXTERN_INLINES 1
#define _EXTERN_INLINE /* Empty to define the real functions.  */
#include "spin-lock.h"

weak_alias (__spin_lock_init, spin_lock_init);
weak_alias (__spin_lock_locked, spin_lock_locked);
weak_alias (__spin_lock, spin_lock);
weak_alias (__spin_unlock, spin_unlock);
weak_alias (__spin_try_lock, spin_try_lock);
