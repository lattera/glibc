/* dlerror -- Return error detail for failing <dlfcn.h> functions.
Copyright (C) 1995 Free Software Foundation, Inc.
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

#include <link.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int _dl_last_errcode;
static const char *_dl_last_errstring;

char *
dlerror (void)
{
 char *ret;

  if (! _dl_last_errstring)
    return NULL;

  if (_dl_last_errcode)
    {
      static char *buf;
      if (buf)
	{
	  free (buf);
	  buf = NULL;
	}
      if (asprintf (&buf, "%s: %s",
		    _dl_last_errstring, strerror (_dl_last_errcode)) == -1)
	return NULL;
      else
	ret = buf;
    }
 else
   ret = (char *) _dl_last_errstring;

 /* Reset the error indicator.  */
 _dl_last_errstring = NULL;
 return ret;
}

int
_dlerror_run (void (*operate) (void))
{
  _dl_last_errcode = _dl_catch_error (&_dl_last_errstring, operate);
  return _dl_last_errstring != NULL;
}
