/* Copyright (C) 1991, 1993, 1995, 1996, 1997, 1998, 2000, 2002, 2004
   Free Software Foundation, Inc.
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

#include <errno.h>
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

/* Fill buf with a string describing the errno code in ERRNUM.  */
int
__xpg_strerror_r (int errnum, char *buf, size_t buflen)
{
  if (errnum < 0 || errnum >= _sys_nerr_internal
      || _sys_errlist_internal[errnum] == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }
  const char *estr = (const char *) _(_sys_errlist_internal[errnum]);
  size_t estrlen = strlen (estr) + 1;

  if (buflen < estrlen)
    {
      __set_errno (ERANGE);
      return -1;
    }

  memcpy (buf, estr, estrlen);
  return 0;
}
