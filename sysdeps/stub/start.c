/* This file should define the low-level program entry point,
   which should set up `__environ', and then do:
     __libc_init(argc, argv, __environ);
     exit(main(argc, argv, __environ));

   This file should be prepared to be the first thing in the text section (on
   Unix systems), or otherwise appropriately special.  */

volatile int errno;

#ifndef HAVE_GNU_LD
#undef environ
#define __environ environ
#endif

char **__environ;
