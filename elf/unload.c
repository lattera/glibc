/* Test for unloading (really unmapping) of objects.  By Franz Sirl.
   This test does not have to passed in all dlopen() et.al. implementation
   since it is not required the unloading actually happens.  But we
   require it for glibc.  */

#include <dlfcn.h>
#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  void *next;
} strct;

int
main (void)
{
   void *sohandle;
   strct *testdat;
   int ret;
   int result = 0;

   mtrace ();

   sohandle = dlopen ("unloadmod.so", RTLD_NOW | RTLD_GLOBAL);
   if (sohandle == NULL)
     {
       printf ("first dlopen failed: %s\n", dlerror ());
       exit (1);
     }

   testdat = dlsym (sohandle, "testdat");
   testdat->next = (void *) -1;

   ret = dlclose (sohandle);
   if (ret != 0)
     {
       puts ("first dlclose failed");
       result = 1;
     }

   sohandle = dlopen ("unloadmod.so", RTLD_NOW | RTLD_GLOBAL);
   if (sohandle == NULL)
     {
       printf ("second dlopen failed: %s\n", dlerror ());
       exit (1);
     }

   testdat = dlsym (sohandle, "testdat");
   if (testdat->next == (void *) -1)
     {
       puts ("testdat->next == (void *) -1");
       result = 1;
     }

   ret = dlclose (sohandle);
   if (ret != 0)
     {
       puts ("second dlclose failed");
       result = 1;
     }

   return result;
}
