#include <dlfcn.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>

int
main (void)
{
  int (*foo) (void);
  void *h;
  int ret;

  h = dlopen ("constload2.so", RTLD_LAZY | RTLD_GLOBAL);
  if (h == NULL)
    error (EXIT_FAILURE, errno, "cannot load module \"constload2.so\"");
  foo = dlsym (h, "foo");
  ret = foo ();
  dlclose (h);
  return ret;
}
