/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <stdlib.h>

long
a64l (string)
     const char *string;
{
  int cnt;
  long result = 0l;

  for (cnt = 0; cnt < 6; ++cnt)
    {
      result <<= 6;
      switch (string[cnt])
	{
	case '.':
	  break;
	case '/':
	  result |= 1;
	  break;
	case '0' ... '9':
	  result |= 2 + string[cnt] - '0';
	  break;
	case 'A' ... 'Z':
	  result |= 12 + string[cnt] - 'A';
	  break;
	case 'a' ... 'z':
	  result |= 38 + string[cnt] - 'a';
	  break;
	default:
	  return result >> 6;
	}
    }

  return result;
}
