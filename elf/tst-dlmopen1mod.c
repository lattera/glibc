#include <dlfcn.h>
#include <stdio.h>
#include <gnu/lib-names.h>


int
foo (Lmid_t ns2)
{
  void *h = dlopen (LIBC_SO, RTLD_LAZY|RTLD_NOLOAD);
  if (h == NULL)
    {
      printf ("cannot get handle for %s: %s\n", LIBC_SO, dlerror ());
      return 1;
    }

  Lmid_t ns = -10;
  if (dlinfo (h, RTLD_DI_LMID, &ns) != 0)
    {
      printf ("dlinfo for %s in %s failed: %s\n",
	      LIBC_SO, __func__, dlerror ());
      return 1;
    }

  if (ns != ns2)
    {
      printf ("namespace for %s not LM_ID_BASE\n", LIBC_SO);
      return 1;
    }

  if (dlclose (h) != 0)
    {
      printf ("dlclose for %s in %s failed: %s\n",
	      LIBC_SO, __func__, dlerror ());
      return 1;
    }

  return 0;
}
