/* Copyright (C) 1996,2001,2004,2006,2007,2010,2011
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <bits/libc-lock.h>

#include <libnsl.h>


/* Path of the file.  */
static const char default_nss[] = "/etc/default/nss";

/* Flags once read from the file.  */
static int default_nss_flags;

/* Code to make sure we call 'init' once.  */
__libc_once_define (static, once);

/* Table of the recognized variables.  */
static const struct
{
  char name[23];
  unsigned int len;
  int flag;
} vars[] =
  {
#define STRNLEN(s) s, sizeof (s) - 1
    { STRNLEN ("NETID_AUTHORITATIVE"), NSS_FLAG_NETID_AUTHORITATIVE },
    { STRNLEN ("SERVICES_AUTHORITATIVE"), NSS_FLAG_SERVICES_AUTHORITATIVE },
    { STRNLEN ("SETENT_BATCH_READ"), NSS_FLAG_SETENT_BATCH_READ },
    { STRNLEN ("ADJUNCT_AS_SHADOW"), NSS_FLAG_ADJUNCT_AS_SHADOW },
  };
#define nvars (sizeof (vars) / sizeof (vars[0]))


static void
init (void)
{
  int saved_errno = errno;
  FILE *fp = fopen (default_nss, "rce");
  if (fp != NULL)
    {
      char *line = NULL;
      size_t linelen = 0;

      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      while (!feof_unlocked (fp))
	{
	  ssize_t n = getline (&line, &linelen, fp);
	  if (n <= 0)
	    break;

	  /* Recognize only

	       <THE-VARIABLE> = TRUE

	     with arbitrary white spaces.  */
	  char *cp = line;
	  while (isspace (*cp))
	    ++cp;

	  /* Recognize comment lines.  */
	  if (*cp == '#')
	    continue;

	  int idx;
	  for (idx = 0; idx < nvars; ++idx)
	    if (strncmp (cp, vars[idx].name, vars[idx].len) == 0)
	      break;
	  if (idx == nvars)
	    continue;

	  cp += vars[idx].len;
	  while (isspace (*cp))
	    ++cp;
	  if (*cp++ != '=')
	    continue;
	  while (isspace (*cp))
	    ++cp;

	  if (strncmp (cp, "TRUE", 4) != 0)
	    continue;
	  cp += 4;

	  while (isspace (*cp))
	    ++cp;

	  if (*cp == '\0')
	    default_nss_flags |= vars[idx].flag;
	}

      free (line);

      fclose (fp);
    }
  __set_errno (saved_errno);
}


int
_nsl_default_nss (void)
{
  /* If we have not yet read the file yet do it now.  */
  __libc_once (once, init);

  return default_nss_flags;
}
