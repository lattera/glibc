#include "posix/sched.h"

/* Now define the internal interfaces.  */

/* This is Linux specific.  */
extern int __clone __P ((int (*__fn) (void *__arg), void *__child_stack,
			 int __flags, void *__arg));
