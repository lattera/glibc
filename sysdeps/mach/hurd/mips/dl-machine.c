/* Operating system support for run-time dynamic linker.  MIPS specific
   stuffs on Hurd.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <hurd.h>
#include <link.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include <sysdep.h>
#include <mach/mig_support.h>
#include "../stdio-common/_itoa.h"
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>
#include <libintl.h>

void weak_function
abort (void)
{
  _exit (127);
}


#include <string.h>
#include <mach/error.h>
#include <errorlib.h>

#undef _
#define _(x) x

/* Return a string describing the errno code in ERRNUM.  */
char * weak_function
_strerror_internal (int errnum, char *buf, size_t buflen)
{
  int system;
  int sub;
  int code;
  const struct error_system *es;
  extern void __mach_error_map_compat (int *);

  __mach_error_map_compat (&errnum);

  system = err_get_system (errnum);
  sub = err_get_sub (errnum);
  code = err_get_code (errnum);

  if (system > err_max_system || ! __mach_error_systems[system].bad_sub)
    {
      const char *unk = _("Error in unknown error system: ");
      const size_t unklen = strlen (unk);
      char *p = buf + buflen;
      *--p = '\0';
      p = _itoa (errnum, p, 16, 1);
      return memcpy (p - unklen, unk, unklen);
    }

  es = &__mach_error_systems[system];

  if (sub >= es->max_sub)
    return (char *) es->bad_sub;

  if (code >= es->subsystem[sub].max_code)
    {
      const char *unk = _("Unknown error ");
      const size_t unklen = strlen (unk);
      char *p = buf + buflen;
      size_t len = strlen (es->subsystem[sub].subsys_name);
      *--p = '\0';
      p = _itoa (errnum, p, 16, 1);
      *p-- = ' ';
      p = memcpy (p - len, es->subsystem[sub].subsys_name, len);
      return memcpy (p - unklen, unk, unklen);
    }

  return (char *) _(es->subsystem[sub].codes[code]);
}

/* Read the whole contents of FILE into new mmap'd space with given
   protections.  The size of the file is returned in SIZE.  */
void *
_dl_sysdep_read_whole_file (const char *file, size_t *size, int prot)
{
  struct stat stat;
  mach_port_t memobj_rd;
  void *contents;
  error_t err;

  memobj_rd = __open (file, O_RDONLY, 0);
  if (memobj_rd)
    {
      err = __io_stat ((file_t) memobj_rd, &stat);
      if (err)
	{
	  __hurd_fail (err);
	  contents = 0;
	}
      else
	{
	  /* Map a copy of the file contents.  */
	  contents = __mmap (0, stat.st_size, prot, MAP_COPY, memobj_rd, 0);
	  if (contents == (void *)-1)
	    contents = 0;
	  else
	    *size = stat.st_size;
	}

      __mach_port_deallocate (__mach_task_self (), memobj_rd);
    }
  else
    contents = 0;

  return contents;
}
