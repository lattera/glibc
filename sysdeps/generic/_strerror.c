/* Copyright (C) 1991,93,95,96,97,98,2000,2002 Free Software Foundation, Inc.
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

#include <libintl.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <stdio-common/_itoa.h>

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
  if (errnum < 0 || errnum >= _sys_nerr_internal
      || _sys_errlist_internal[errnum] == NULL)
    {
      /* Buffer we use to print the number in.  For a maximum size for
	 `int' of 8 bytes we never need more than 20 digits.  */
      char numbuf[21];
      const char *unk = _("Unknown error ");
      const size_t unklen = strlen (unk);
      char *p, *q;

      numbuf[20] = '\0';
      p = _itoa_word (errnum, &numbuf[20], 10, 0);

      /* Now construct the result while taking care for the destination
	 buffer size.  */
      q = __mempcpy (buf, unk, MIN (unklen, buflen));
      if (unklen < buflen)
	memcpy (q, p, MIN ((size_t) (&numbuf[21] - p), buflen - unklen));

      /* Terminate the string in any case.  */
      if (buflen > 0)
	buf[buflen - 1] = '\0';

      return buf;
    }

  return (char *) _(_sys_errlist_internal[errnum]);
}
weak_alias (__strerror_r, strerror_r)
libc_hidden_def (__strerror_r)
