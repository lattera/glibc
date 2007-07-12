#include <stdio.h>
#include <dlfcn.h>
#include <mcheck.h>
#include <stdlib.h>

int
main (void)
{
  mtrace ();

  int ret = 0;
  for (int i = 0; i < 10; i++)
    {
      void *h = dlopen (i < 5 ? "./tst-leaks1.c"
			      : "$ORIGIN/tst-leaks1.o", RTLD_LAZY);
      if (h != NULL)
	{
	  puts ("dlopen unexpectedly succeeded");
	  ret = 1;
	  dlclose (h);
	}
    }

  return ret;
}
