#include <dlfcn.h>

int
successful_rtld_next_test (void)
{
  int (*fp) (void);

  /* Get the next function... */
  fp = (int (*) (void)) dlsym (RTLD_NEXT, __FUNCTION__);

  /* ...and simply call it.  */
  return fp ();
}


void *
failing_rtld_next_use (void)
{
  return dlsym (RTLD_NEXT, __FUNCTION__);
}
