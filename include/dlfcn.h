#ifndef _DLFCN_H
#include <elf/dlfcn.h>

/* Now define the internal interfaces.  */
extern void *__dlvsym __P ((void *__handle, __const char *__name,
			    __const char *__version));

extern void *__libc_dlopen  __P ((__const char *__name));
extern void *__libc_dlsym   __P ((void *__map, __const char *__name));
extern int   __libc_dlclose __P ((void *__map));
extern int   _dl_addr __P ((const void *address, Dl_info *info))
    internal_function;

#endif
