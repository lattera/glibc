/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "utmpd-private.h"


/* This variable indicates if we have forked.  If set, we log messages
   via the system logger.  Otherwise we simply print the program name
   and the message to standard error.  */
int forked = 0;


/* Log error message MESSAGE, which is a printf-style format string
   with optional args.
   If ERRNUM is nonzero, also log its corresponding system error message.
   Exit with status STATUS if it is nonzero.  */
void
error (int status, int errnum, const char *message, ...)
{
  va_list ap;
  char *buffer = NULL;

  va_start (ap, message);
  vasprintf (&buffer, message, ap);
  va_end (ap);

  if (forked)
    {
      if (errnum == 0)
	syslog (LOG_ERR, "%s", buffer);
      else
	syslog (LOG_ERR, "%s: %s", buffer, strerror (errnum));
    }
  else
    {
      if (errnum == 0)
	fprintf (stderr, "%s: %s\n", program_invocation_name, buffer);
      else
	fprintf (stderr, "%s: %s: %s\n", program_invocation_name, buffer,
		 strerror (errnum));
    }

  if (buffer)
    free (buffer);

  if (status)
    exit (status);
}

/* Log warning message MESSAGE, which is a printf-style format string
   with optional args.
   If ERRNUM is nonzero, also log its corresponding system error message. */
void
warning (int errnum, const char *message, ...)
{
  va_list ap;
  char *buffer = NULL;

  va_start (ap, message);
  vasprintf (&buffer, message, ap);
  va_end (ap);

  if (forked)
    {
      if (errnum == 0)
	syslog (LOG_WARNING, "%s", buffer);
      else
	syslog (LOG_WARNING, "%s: %s", buffer, strerror (errnum));
    }
  else
    {
      if (errnum == 0)
	printf ("%s: %s\n", program_invocation_name, buffer);
      else
	printf ("%s: %s: %s\n", program_invocation_name, buffer,
		strerror (errnum));
    }

  if (buffer)
    free (buffer);
}
