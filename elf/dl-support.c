/* Support for dynamic linking code in static libc.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* This file defines some things that for the dynamic linker are defined in
   rtld.c and dl-sysdep.c in ways appropriate to bootstrap dynamic linking.  */

#include <stdlib.h>


extern char *__progname;
char **_dl_argv = &__progname;	/* This is checked for some error messages.  */

/* This defines the default search path for libraries.
   For the dynamic linker it is set by -rpath when linking.  */
const char *_dl_rpath = DEFAULT_RPATH;

/* If nonzero print warnings about problematic situations.  */
int _dl_verbose;


static void init_verbose (void) __attribute__ ((unused));

static void
init_verbose (void)
{
  _dl_verbose = *(getenv ("LD_WARN") ?: "") == '\0' ? 0 : 1;
}
text_set_element (__libc_subinit, init_verbose);
