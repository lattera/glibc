/* Startup support for ELF initializers/finalizers in the main executable.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stddef.h>

#ifdef HAVE_INITFINI_ARRAY
/* These magic symbols are provided by the linker.  */
extern void (*__preinit_array_start []) (void) attribute_hidden;
extern void (*__preinit_array_end []) (void) attribute_hidden;
extern void (*__init_array_start []) (void) attribute_hidden;
extern void (*__init_array_end []) (void) attribute_hidden;
extern void (*__fini_array_start []) (void) attribute_hidden;
extern void (*__fini_array_end []) (void) attribute_hidden;
#endif

/* These function symbols are provided for the .init/.fini section entry
   points automagically by the linker.  */
extern void _init (void);
extern void _fini (void);

/* These functions are passed to __libc_start_main by the startup code.
   These get statically linked into each program.  For dynamically linked
   programs, this module will come from libc_nonshared.a and differs from
   the libc.a module in that it doesn't call the preinit array.  */

void
__libc_csu_init (void)
{
#ifdef HAVE_INITFINI_ARRAY
  /* For dynamically linked executables the preinit array is executed by
     the dynamic linker (before initializing any shared object.  */

# ifndef LIBC_NONSHARED
  /* For static executables, preinit happens rights before init.  */
  {
    const size_t size = __preinit_array_end - __preinit_array_start;
    size_t i;
    for (i = 0; i < size; i++)
      (*__preinit_array_start [i]) ();
  }
# endif
#endif

  _init ();

#ifdef HAVE_INITFINI_ARRAY
  {
    const size_t size = __init_array_end - __init_array_start;
    size_t i;
    for (i = 0; i < size; i++)
      (*__init_array_start [i]) ();
  }
#endif
}

void
__libc_csu_fini (void)
{
#ifdef HAVE_INITFINI_ARRAY
  size_t i = __fini_array_end - __fini_array_start;
  while (i-- > 0)
    (*__fini_array_start [i]) ();
#endif

  _fini ();
}
