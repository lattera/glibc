/* Initializer module for building the ELF shared C library.  This file and
   sofini.c do the work normally done by crtbeginS.o and crtendS.o, to wrap
   the `.ctors' and `.dtors' sections so the lists are terminated, and
   calling those lists of functions.  */

#include <libc-internal.h>
#include <stdlib.h>

#ifdef HAVE_DWARF2_UNWIND_INFO_STATIC
# include <gccframe.h>
#endif

static void (*const __CTOR_LIST__[1]) (void)
     __attribute__ ((section (".ctors")))
     = { (void (*) (void)) -1 };
static void (*const __DTOR_LIST__[1]) (void)
     __attribute__ ((section (".dtors")))
     = { (void (*) (void)) -1 };

static inline void
run_hooks (void (*const list[]) (void))
{
  while (*++list)
    (**list) ();
}

#ifdef HAVE_DWARF2_UNWIND_INFO
static char __EH_FRAME_BEGIN__[]
     __attribute__ ((section (".eh_frame")))
     = { };
# ifdef HAVE_DWARF2_UNWIND_INFO_STATIC
extern void __register_frame_info (const void *, struct object *);
extern void __register_frame_info_bases (const void *, struct object *,
					 void *, void *);
extern void __deregister_frame_info (const void *);
extern void __deregister_frame_info_bases (const void *);
# else
extern void __register_frame (const void *);
extern void __deregister_frame (const void *);
# endif
#endif

/* This function will be called from _init in init-first.c.  */
void
__libc_global_ctors (void)
{
  /* Call constructor functions.  */
  run_hooks (__CTOR_LIST__);

#ifdef HAVE_DWARF2_UNWIND_INFO
# ifdef HAVE_DWARF2_UNWIND_INFO_STATIC
  {
    static struct object ob;
#  if defined CRT_GET_RFIB_TEXT || defined CRT_GET_RFIB_DATA
    void *tbase, *dbase;

#   ifdef CRT_GET_RFIB_TEXT
    CRT_GET_RFIB_TEXT (tbase);
#   else
    tbase = NULL;
#   endif
#   ifdef CRT_GET_RFIB_DATA
    CRT_GET_RFIB_DATA (dbase);
#   else
    dbase = NULL;
#   endif
    __register_frame_info_bases (__EH_FRAME_BEGIN__, &ob, tbase, dbase);
#  else
    __register_frame_info (__EH_FRAME_BEGIN__, &ob);
#  endif
  }
# else
  __register_frame (__EH_FRAME_BEGIN__);
# endif
#endif
}


/* This function becomes the DT_FINI termination function
   for the C library.  */
#ifndef HAVE_INITFINI_ARRAY
void _fini (void) __attribute__ ((section (".fini"))); /* Just for kicks.  */
void
_fini (void)
#else
void
__libc_fini (void)
#endif
{
  /* Call destructor functions.  */
  run_hooks (__DTOR_LIST__);
#ifdef HAVE_DWARF2_UNWIND_INFO
# ifdef HAVE_DWARF2_UNWIND_INFO_STATIC
#  if defined CRT_GET_RFIB_TEXT || defined CRT_GET_RFIB_DATA
  __deregister_frame_info_bases (__EH_FRAME_BEGIN__);
#  else
  __deregister_frame_info (__EH_FRAME_BEGIN__);
#  endif
# else
  __deregister_frame (__EH_FRAME_BEGIN__);
# endif
#endif
}
#ifdef HAVE_INITFINI_ARRAY
void (*_fini_ptr) (void) __attribute__ ((section (".fini_array")))
     = &__libc_fini;
#endif
