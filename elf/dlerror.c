/* Return error detail for failing <dlfcn.h> functions.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <link.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int last_errcode;
static char *last_errstring;
static const char *last_object_name;

char *
dlerror (void)
{
  static char *buf;
  char *ret;

  if (buf)
    {
      free (buf);
      buf = NULL;
    }

  if (! last_errstring)
    return NULL;

  if (last_errcode == 0 && ! last_object_name)
    ret = (char *) last_errstring;
  else if (last_errcode == 0)
    ret = (asprintf (&buf, "%s: %s", last_object_name, last_errstring) == -1
	   ? NULL : buf);
  else if (! last_object_name)
    ret = (asprintf (&buf, "%s: %s",
		     last_errstring, strerror (last_errcode)) == -1
	   ? NULL : buf);
  else
    ret = (asprintf (&buf, "%s: %s: %s",
		     last_object_name, last_errstring,
		     strerror (last_errcode)) == -1
	   ? NULL : buf);

  /* Reset the error indicator.  */
  free (last_errstring);
  last_errstring = NULL;
  return ret;
}

int
_dlerror_run (void (*operate) (void *), void *args)
{
  if (last_errstring != NULL)
    /* Free the error string from the last failed command.  This can
       happen if `dlerror' was not run after an error was found.  */
    free (last_errstring);

  last_errcode = _dl_catch_error (&last_errstring, &last_object_name,
				  operate, args);
  return last_errstring != NULL;
}
