#include_next <internaltypes.h>
#ifndef _INTERNAL_TYPES_H_HPPA_ 
#define _INTERNAL_TYPES_H_HPPA_ 1
#include <atomic.h>

/* In GLIBC 2.10 HPPA switched from Linuxthreads to NPTL, and in order 
to maintain ABI compatibility with pthread_cond_t, some care had to be
taken.

The NPTL pthread_cond_t grew in size. When HPPA switched to NPTL, we
dropped the use of ldcw, and switched to the kernel helper routine for
compare-and-swap.  This allowed HPPA to use the 4-word 16-byte aligned
lock words, and alignment words to store the additional pthread_cond_t
data. Once organized properly the new NPTL pthread_cond_t was 1 word
smaller than the Linuxthreads version.

However, we were faced with the case that users may have initialized the
pthread_cond_t with PTHREAD_COND_INITIALIZER. In this case, the first
four words were set to one, and must be cleared before any NPTL code
used these words.

We didn't want to use LDCW, because it continues to be a source of bugs
when applications memset pthread_cond_t to all zeroes by accident. This
works on all other architectures where lock words are unlocked at zero.
Remember that because of the semantics of LDCW, a locked word is set to
zero, and an unlocked word is set to 1.

Instead we used atomic_compare_and_exchange_val_acq, but we couldn't use
this on any of the pthread_cond_t words, otherwise it might interfere
with the current operation of the structure. To solve this problem we
used the left over word.

If the stucture was initialized by a legacy Linuxthread
PTHREAD_COND_INITIALIZER it contained a 1, and this indicates that the
structure requires zeroing for NPTL. The first thread to come upon a
pthread_cond_t with a 1 in the __initializer field, will
compare-and-swap the value, placing a 2 there which will cause all other
threads using the same pthread_cond_t to wait for the completion of the
initialization. Lastly, we use a store (with memory barrier) to change
__initializer from 2 to 0. Note that the store is strongly ordered, but
we use the PA 1.1 compatible form which is ",ma" with zero offset.

In the future, when the application is recompiled with NPTL
PTHREAD_COND_INITIALIZER it will be a quick compare-and-swap, which
fails because __initializer is zero, and the structure will be used as
is correctly.  */

#define cond_compat_clear(var) \
({											\
  int tmp = 0;										\
  var->__data.__lock = 0;								\
  var->__data.__futex = 0;								\
  var->__data.__mutex = NULL;								\
  /* Clear __initializer last, to indicate initialization is done.  */			\
  __asm__ __volatile__ ("stw,ma %1,0(%0)"						\
			: : "r" (&var->__data.__initializer), "r" (tmp) : "memory");	\
})

#define cond_compat_check_and_clear(var) \
({								\
  int ret;							\
  volatile int *value = &var->__data.__initializer;		\
  if ((ret = atomic_compare_and_exchange_val_acq(value, 2, 1)))	\
    {								\
      if (ret == 1)						\
	{							\
	  /* Initialize structure.  */				\
	  cond_compat_clear (var);				\
	}							\
      else							\
        {							\
	  /* Yield until structure is initialized.  */		\
	  while (*value == 2) sched_yield ();			\
        }							\
    }								\
})

#endif

