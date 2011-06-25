/* Initializer module for building the ELF shared C library.  This file and
   sofini.c do the work normally done by crtbeginS.o and crtendS.o, to wrap
   the `.ctors' and `.dtors' sections so the lists are terminated, and
   calling those lists of functions.  */

#ifndef NO_CTORS_DTORS_SECTIONS
# include <libc-internal.h>
# include <stdlib.h>

static void (*const __CTOR_LIST__[1]) (void)
  __attribute__ ((used, section (".ctors")))
  = { (void (*) (void)) -1 };
static void (*const __DTOR_LIST__[1]) (void)
  __attribute__ ((used, section (".dtors")))
  = { (void (*) (void)) -1 };

static inline void
run_hooks (void (*const list[]) (void))
{
  while (*++list)
    (**list) ();
}

/* This function will be called from _init in init-first.c.  */
void
__libc_global_ctors (void)
{
  /* Call constructor functions.  */
  run_hooks (__CTOR_LIST__);
}


/* This function becomes the DT_FINI termination function
   for the C library.  */
void
__libc_fini (void)
{
  /* Call destructor functions.  */
  run_hooks (__DTOR_LIST__);
}

void (*_fini_ptr) (void) __attribute__ ((section (".fini_array")))
     = &__libc_fini;
#endif
