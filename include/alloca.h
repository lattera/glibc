#include <stdlib/alloca.h>

#undef	__alloca

/* Now define the internal interfaces.  */
extern __ptr_t __alloca __P ((size_t __size));

#ifdef	__GNUC__
# define __alloca(size)	__builtin_alloca (size)
#endif /* GCC.  */
