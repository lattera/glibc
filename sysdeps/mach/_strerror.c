/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <string.h>
#include <mach/error.h>
#include <errorlib.h>
#include "../stdio/_itoa.h"

/* Return a string describing the errno code in ERRNUM.  */
char *
_strerror_internal (int errnum, char buf[1024])
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
      static const char unk[] = "Error in unknown error system: ";
      char *p = buf + sizeof buf;
      *p-- = '\0';
      p = _itoa (errnum, p, 16, 1);
      p -= sizeof unk - 1;
      return memcpy (p, unk, sizeof unk - 1);
    }

  es = &__mach_error_systems[system];

  if (sub >= es->max_sub)
    return (char *) es->bad_sub;

  if (code >= es->subsystem[sub].max_code)
    {
      static const char unk[] = "Unknown error ";
      char *p = buf + sizeof buf;
      size_t len = strlen (es->subsystem[sub].subsys_name);
      *p-- = '\0';
      p = _itoa (errnum, p, 16, 1);
      *p-- = ' ';
      p = memcpy (p - len, es->subsystem[sub].subsys_name, len);
      return memcpy (p - sizeof unk - 1, unk, sizeof unk - 1);
    }

  return (char *) es->subsystem[sub].codes[code];
}
