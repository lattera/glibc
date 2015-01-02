/* strerror_l - Get errno description string in given locale.  Mach version.
   Copyright (C) 2007-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <libintl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach/error.h>
#include <errorlib.h>
#include <sys/param.h>


static __thread char *last_value;


static const char *
translate (const char *str, locale_t loc)
{
  locale_t oldloc = __uselocale (loc);
  const char *res = _(str);
  __uselocale (oldloc);
  return res;
}


/* Return a string describing the errno code in ERRNUM.  */
char *
strerror_l (int errnum, locale_t loc)
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
      free (last_value);
      if (__asprintf (&last_value, "%s%X",
		      translate ("Error in unknown error system: ", loc),
		      errnum) == -1)
	last_value = NULL;

      return last_value;
    }

  es = &__mach_error_systems[system];

  if (sub >= es->max_sub)
    return (char *) translate (es->bad_sub, loc);

  if (code >= es->subsystem[sub].max_code)
    {
      free (last_value);
      if (__asprintf (&last_value, "%s%s %d",
		      translate ("Unknown error ", loc),
		      translate (es->subsystem[sub].subsys_name, loc),
		      errnum) == -1)
	last_value = NULL;

      return last_value;
    }

  return (char *) translate (es->subsystem[sub].codes[code], loc);
}


#ifdef _LIBC
# ifdef _LIBC_REENTRANT
/* This is called when a thread is exiting to free the last_value string.  */
static void __attribute__ ((section ("__libc_thread_freeres_fn")))
strerror_thread_freeres (void)
{
  free (last_value);
}
text_set_element (__libc_thread_subfreeres, strerror_thread_freeres);
text_set_element (__libc_subfreeres, strerror_thread_freeres);
# endif
#endif
