#ifndef _ALLOCA_H

#include <stdlib/alloca.h>

#undef	__alloca

/* Now define the internal interfaces.  */
extern void *__alloca (size_t __size);

#ifdef	__GNUC__
# define __alloca(size)	__builtin_alloca (size)
#endif /* GCC.  */

extern int __libc_use_alloca (size_t size) __attribute__ ((const));
extern int __libc_alloca_cutoff (size_t size) __attribute__ ((const));

#define __MAX_ALLOCA_CUTOFF	65536

#include <allocalim.h>

#endif
