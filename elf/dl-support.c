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

#include <link.h>
#include <stdlib.h>
#include <unistd.h>
#include <dl-machine.h>

extern char *__progname;
char **_dl_argv = &__progname;	/* This is checked for some error messages.  */

/* This defines the default search path for libraries.
   For the dynamic linker it is set by -rpath when linking.  */
const char *_dl_rpath = DEFAULT_RPATH;

/* Name of the architecture.  */
const char *_dl_platform;
size_t _dl_platformlen;

/* If nonzero print warnings about problematic situations.  */
int _dl_verbose;

/* Structure to store information about search paths.  */
struct r_search_path *_dl_search_paths;


static void non_dynamic_init (void) __attribute__ ((unused));

static void
non_dynamic_init (void)
{
  _dl_verbose = *(getenv ("LD_WARN") ?: "") == '\0' ? 0 : 1;

  /* Initialize the data structures for the search paths for shared
     objects.  */
  _dl_init_paths ();

#ifdef DL_PLATFORM_INIT
  DL_PLATFORM_INIT;
#endif

  /* Now determine the length of the platform string.  */
  if (_dl_platform != NULL)
    _dl_platformlen = strlen (_dl_platform);

  _dl_pagesize = __getpagesize ();
}
text_set_element (__libc_subinit, non_dynamic_init);
