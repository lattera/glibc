/* Copyright (C) 1991 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <localeinfo.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


/* Transform SRC into a form such that the result of strcmp
   on two strings that have been transformed by strxfrm is
   the same as the result of strcoll on the two strings before
   their transformation.  The transformed string is put in at
   most N characters of DEST and its length is returned.  */
size_t
DEFUN(strxfrm, (dest, src, n), char *dest AND CONST char *src AND size_t n)
{
  CONST unsigned char *CONST values
    = _collate_info != NULL ? _collate_info->values : NULL;
  CONST unsigned char *CONST offsets
    = _collate_info != NULL ? _collate_info->offsets : NULL;
  register size_t done = 0;

  while (*src != '\0')
    {
      CONST unsigned char c = *src++;

      ++done;
      if (offsets != NULL && offsets[c] != 0)
	{
	  ++done;
	  if (offsets[c] == CHAR_MAX)
	    ++done;
	}
      if (done < n && dest != NULL)
	{
	  if (values == NULL)
	    *dest++ = c;
	  else if (values[c] == UCHAR_MAX && offsets[c] == 0)
	    /* This is a non-collating element.  Skip it.  */
	    ;
	  else if (values[c] == UCHAR_MAX && offsets[c] == CHAR_MAX)
	    {
	      /* This element collates lower than anything else.  */
	      *dest++ = '\001';
	      *dest++ = '\001';
	      *dest++ = '\001';
	    }
	  else
	    {
	      *dest++ = values[c];
	      *dest++ = offsets[c];
	    }
	}
    }

  if (dest != NULL)
    *dest = '\0';
  return done;
}
