/* Copyright (c) 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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

#include <stdarg.h>
#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include "dbg_log.h"
#include "nscd.h"

/* if in debug mode and we have a debug file, we write the messages to it,
   if in debug mode and no debug file, we write the messages to stderr,
   else to syslog.  */

FILE *dbgout;
int debug_level;

int
set_logfile (const char *logfile)
{
  dbgout = fopen (logfile, "a");
  return dbgout == NULL ? 0 : 1;
}

void
dbg_log (const char *fmt,...)
{
  va_list ap;
  char msg[512], msg2[512];

  va_start (ap, fmt);
  vsnprintf (msg2, sizeof (msg), fmt, ap);

  if (debug_level > 0)
    {
      snprintf (msg, sizeof (msg), "%d: %s\n", getpid (), msg2);
      if (dbgout)
	{
	  fputs (msg, dbgout);
	  fflush (dbgout);
	}
      else
	fputs (msg, stderr);
    }
  else
    {
      snprintf (msg, sizeof (msg), "%d: %s", getpid (), msg2);
      syslog (LOG_NOTICE, "%s", msg);
    }
  va_end (ap);
}
