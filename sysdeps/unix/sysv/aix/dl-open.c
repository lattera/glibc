/* XXX The implementation of dlopen should somehow use the __loadx system
   call but how?  */
#include <dlfcn.h>

void *
__libc_dlopen (const char *file)
{
  return (void *) 0;
}
