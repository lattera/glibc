/* XXX The implementation of dlopen should somehow use the __loadx system
   call but how?  */
#include <dlfcn.h>

int
__libc_dlclose (void *handle)
{
  return 0;
}
