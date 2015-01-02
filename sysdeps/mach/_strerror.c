/* Copyright (C) 1993-2015 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <string.h>
#include <mach/error.h>
#include <errorlib.h>
#include <sys/param.h>
#include <_itoa.h>

/* It is critical here that we always use the `dcgettext' function for
   the message translation.  Since <libintl.h> only defines the macro
   `dgettext' to use `dcgettext' for optimizing programs this is not
   always guaranteed.  */
#ifndef dgettext
# include <locale.h>		/* We need LC_MESSAGES.  */
# define dgettext(domainname, msgid) dcgettext (domainname, msgid, LC_MESSAGES)
#endif

/* Return a string describing the errno code in ERRNUM.  */
char *
__strerror_r (int errnum, char *buf, size_t buflen)
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
      /* Buffer we use to print the number in.  For a maximum size for
	 `int' of 8 bytes we never need more than 20 digits.  */
      char numbuf[21];
      const char *unk = _("Error in unknown error system: ");
      const size_t unklen = strlen (unk);
      char *p, *q;

      numbuf[20] = '\0';
      p = _itoa_word (errnum, &numbuf[20], 16, 1);

      /* Now construct the result while taking care for the destination
	 buffer size.  */
      q = __mempcpy (buf, unk, MIN (unklen, buflen));
      if (unklen < buflen)
	memcpy (q, p, MIN (&numbuf[21] - p, buflen - unklen));

      /* Terminate the string in any case.  */
      if (buflen > 0)
	buf[buflen - 1] = '\0';

      return buf;
    }

  es = &__mach_error_systems[system];

  if (sub >= es->max_sub)
    return (char *) es->bad_sub;

  if (code >= es->subsystem[sub].max_code)
    {
      /* Buffer we use to print the number in.  For a maximum size for
	 `int' of 8 bytes we never need more than 20 digits.  */
      char numbuf[21];
      const char *unk = _("Unknown error ");
      const size_t unklen = strlen (unk);
      char *p, *q;
      size_t len = strlen (es->subsystem[sub].subsys_name);

      numbuf[20] = '\0';
      p = _itoa_word (errnum, &numbuf[20], 10, 0);

      /* Now construct the result while taking care for the destination
	 buffer size.  */
      q = __mempcpy (buf, unk, MIN (unklen, buflen));
      if (unklen < buflen)
	{
	  q = __mempcpy (q, es->subsystem[sub].subsys_name,
			 MIN (len, buflen - unklen));
	  if (unklen + len < buflen)
	    {
	      *q++ = ' ';
	      if (unklen + len + 1 < buflen)
		memcpy (q, p,
			MIN (&numbuf[21] - p, buflen - unklen - len - 1));
	    }
	}

       /* Terminate the string in any case.  */
      if (buflen > 0)
	buf[buflen - 1] = '\0';

      return buf;
    }

  return (char *) _(es->subsystem[sub].codes[code]);
}
libc_hidden_def (__strerror_r)
weak_alias (__strerror_r, strerror_r)
