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

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* This file defines some things that for the dynamic linker are defined in
   rtld.c and dl-sysdep.c in ways appropriate to bootstrap dynamic linking.  */

extern char *__progname;
char **_dl_argv = &__progname;	/* This is checked for some error messages.  */

/* This defines the default search path for libraries.
   For the dynamic linker it is set by -rpath when linking.  */
const char *_dl_rpath = DEFAULT_RPATH;

#ifndef MAP_ANON
/* This is the only dl-sysdep.c function that is actually needed at run-time
   by _dl_map_object.  */

int
_dl_sysdep_open_zero_fill (void)
{
  return __open ("/dev/zero", O_RDONLY);
}
#endif

/* This should never be called.  */
void
_dl_sysdep_fatal (void)
{
  assert (! "_dl_sysdep_fatal called");
}

/* Read the whole contents of FILE into new mmap'd space with given
   protections.  *SIZEP gets the size of the file.  */

void *
_dl_sysdep_read_whole_file (const char *file, size_t *sizep, int prot)
{
  void *result;
  struct stat st;
  int fd = __open (file, O_RDONLY);
  if (fd < 0)
    return NULL;
  if (__fxstat (_STAT_VER, fd, &st) < 0)
    result = NULL;
  else
    {
      /* Map a copy of the file contents.  */
      result = __mmap (0, st.st_size, prot,
#ifdef MAP_COPY
                       MAP_COPY
#else
                       MAP_PRIVATE
#endif
#ifdef MAP_FILE
                       | MAP_FILE
#endif
                       , fd, 0);
      if (result == (void *) -1)
        result = NULL;
      else
        *sizep = st.st_size;
    }
  __close (fd);
  return result;
}
