/* Generic conversion to and from 8bit charsets.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#define FROM_LOOP		from_generic
#define TO_LOOP			to_generic
#define DEFINE_INIT		1
#define DEFINE_FINI		1
#define MIN_NEEDED_FROM		1
#define MIN_NEEDED_TO		4


/* First define the conversion function from the 8bit charset to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = to_ucs4[*inptr];					      \
									      \
    if (HAS_HOLES && ch == L'\0' && *inptr != '\0')			      \
      {									      \
	/* This is an illegal character.  */				      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
									      \
    *((uint32_t *) outptr)++ = ch;					      \
    ++inptr;								      \
  }
#include <iconv/loop.c>


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    uint32_t ch = *((uint32_t *) inptr);				      \
									      \
    if (ch >= sizeof (from_ucs4) / sizeof (from_ucs4[0])		      \
	|| (ch != 0 && from_ucs4[ch] == '\0'))				      \
      {									      \
	/* This is an illegal character.  */				      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
									      \
    *outptr++ = from_ucs4[ch];						      \
    inptr += 4;								      \
  }
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
