#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>


/* How many load/unload operations do we do.  */
#define TEST_ROUNDS	100


static struct
{
  /* Name of the module.  */
  const char *name;
  /* The handle.  */
  void *handle;
} testobjs[] =
{
  { "testobj1.so", NULL },
  { "testobj2.so", NULL },
  { "testobj3.so", NULL },
};
#define NOBJS	(sizeof (testobjs) / sizeof (testobjs[0]))


static const struct
{
  /* Name of a function to call.  */
  const char *fname;
  /* Index in status and handle array.  */
  int index;
  /* Options while loading the module.  */
  int options;
} tests[] =
{
  { "obj1func2", 0, RTLD_LAZY },
  { "obj1func1", 0, RTLD_LAZY | RTLD_GLOBAL },
  { "obj1func1", 0, RTLD_NOW, },
  { "obj1func2", 0, RTLD_NOW | RTLD_GLOBAL },
  { "obj2func2", 1, RTLD_LAZY },
  { "obj2func1", 1, RTLD_LAZY | RTLD_GLOBAL, },
  { "obj2func1", 1, RTLD_NOW, },
  { "obj2func2", 1, RTLD_NOW | RTLD_GLOBAL },
  { "obj3func2", 2, RTLD_LAZY },
  { "obj3func1", 2, RTLD_LAZY | RTLD_GLOBAL },
  { "obj3func1", 2, RTLD_NOW },
  { "obj3func2", 2, RTLD_NOW | RTLD_GLOBAL },
};
#define NTESTS	(sizeof (tests) / sizeof (tests[0]))


int
main (void)
{
  int count = TEST_ROUNDS;

  /* Just a seed.  */
  srandom (TEST_ROUNDS);

  while (count--)
    {
      int nr = random () % NTESTS;
      int index = tests[nr].index;

      printf ("%4d: %4d: ", count + 1, nr);
      fflush (stdout);

      if (testobjs[index].handle == NULL)
	{
	  int (*fct) (int);

	  /* Load the object.  */
	  testobjs[index].handle = dlopen (testobjs[index].name,
					   tests[nr].options);
	  if (testobjs[index].handle == NULL)
	    error (EXIT_FAILURE, 0, "cannot load `%s': %s",
		   testobjs[index].name, dlerror ());

	  /* Test the function call.  */
	  fct = dlsym (testobjs[index].handle, tests[nr].fname);
	  if (fct == NULL)
	    error (EXIT_FAILURE, 0,
		   "cannot get function `%s' from shared object `%s': %s",
		   tests[nr].fname, testobjs[index].name, dlerror ());

	  fct (10);

	  printf ("successfully loaded `%s'\n", testobjs[index].name);
	}
      else
	{
	  dlclose (testobjs[index].handle);
	  testobjs[index].handle = NULL;

	  printf ("successfully unloaded `%s'\n", testobjs[index].name);
	}
    }

  /* Unload all loaded modules.  */
  for (count = 0; count < NOBJS; ++count)
    if (testobjs[count].handle != NULL)
      dlclose (testobjs[count].handle);

  return 0;
}


int
foo (int a)
{
  return a - 1;
}
