/* Support for dynamic linking code in static libc.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

/* This file defines some things that for the dynamic linker are defined in
   rtld.c and dl-sysdep.c in ways appropriate to bootstrap dynamic linking.  */

int _dl_secure;			/* Always honor LD_LIBRARY_PATH.  */

extern char *__progname;
char **_dl_argv = &__progname;	/* This is checked for some error messages.  */

/* This defines the default search path for libraries.
   For the dynamic linker it is set by -rpath when linking.  */
const char *_dl_rpath = DEFAULT_RPATH;

/* This is the only dl-sysdep.c function that is actually needed at run-time
   by _dl_map_object.  */

int
_dl_sysdep_open_zero_fill (void)
{
  return __open ("/dev/zero", O_RDONLY);
}

/* This should never be called.  */
void
_dl_sysdep_fatal (void)
{
  assert (! "_dl_sysdep_fatal called");
}

