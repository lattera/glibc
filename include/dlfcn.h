#ifndef _DLFCN_H
#include <elf/dlfcn.h>

/* Now define the internal interfaces.  */
extern void *__dlvsym __P ((void *__handle, __const char *__name,
			    __const char *__version));
#endif
