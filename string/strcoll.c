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


/* Compare S1 and S2, returning less than, equal to or
   greater than zero if the collated form of S1 is lexiographically
   less than, equal to or greater than the collated form of S2.  */
int
DEFUN(strcoll, (s1, s2), CONST char *s1 AND CONST char *s2)
{
  if (_collate_info == NULL || _collate_info->values == NULL)
    return strcmp(s1, s2);
  else
    {
      CONST unsigned char *CONST values = _collate_info->values;
      CONST unsigned char *CONST offsets = _collate_info->offsets;

      while (*s1 != '\0' && *s2 != '\0')
	{
	  CONST unsigned char c1 = *s1++, c2 = *s2++;
	  CONST unsigned char v1 = values[c1], v2 = values[c2];
	  CONST unsigned char o1 = offsets[c1], o2 = offsets[c2];

	  if (v1 == UCHAR_MAX && o1 == 0)
	    /* This is a non-collating element.  Skip it.  */
	    --s2;
	  else if (v2 == UCHAR_MAX && o2 == 0)
	    --s1;
	  else if (v1 == UCHAR_MAX && o1 == CHAR_MAX)
	    {
	      /* This element collates lower than anything else.  */
	      if (v2 != UCHAR_MAX || o2 != CHAR_MAX)
		return -1;
	    }
	  else if (v2 == UCHAR_MAX && o2 == CHAR_MAX)
	    return 1;
	  else if (v1 != v2)
	    return v1 - v2;
	  else if (o1 != o2)
	    return o1 - o2;
	}

      if (*s1 == '\0')
	return *s2 == '\0' ? 0 : -1;
      else if (*s2 == '\0')
	return 1;
      return 0;
    }
}
