/* XXX The implementation of dlopen should somehow use the __loadx system
   call but how?  */
#include <dlfcn.h>

void *
__libc_dlsym (void *handle, const char *name)
{
  return (void *) 0;
}
