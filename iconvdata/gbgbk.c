/* Mapping tables from GBK to GB2312 and vice versa.
   Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

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

#include <gconv.h>
#include <stdint.h>


/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME		"GBK//"
#define FROM_LOOP		from_gbk_to_gb
#define TO_LOOP			from_gb_to_gbk
#define DEFINE_INIT		1
#define DEFINE_FINI		1
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		2
#define MIN_NEEDED_TO		1
#define MAX_NEEDED_TO		2


/* First define the conversion function from GBK to GB2312.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = *inptr;						      \
									      \
    if (ch <= 0x7f)							      \
      *outptr++ = *inptr++;						      \
    else								      \
      {									      \
	/* It's a two-byte sequence.  We have to mask out all the sequences   \
	   which are not in GB2312.  Besides all of them in the range	      \
	   0x8140 to 0xA0FE this also includes in the remaining range the     \
	   sequences which the second byte being in the range from 0x40 to    \
	   0xA0 and the following exceptions:				      \
									      \
	     0xA2A1 to 0xA2A9,						      \
	     0xA2AA,							      \
	     0xA6E0 to 0xA6EB,						      \
	     0xA6EE to 0xA6F2,						      \
	     0xA6F4, 0xA6F5,						      \
	     0xA8BB to 0xA8C0						      \
									      \
	   All these characters are not defined in GB2312.  Besides this      \
	   there is an incomatibility in the mapping.  The Unicode tables     \
	   say that 0xA1A4 maps in GB2312 to U30FB while in GBK it maps to    \
	   U00B7.  Since we are free to do whatever we want if a mapping      \
	   is not available we will not flag this as an error but instead     \
	   map the two positions.  But this means that the mapping	      \
									      \
		UCS4 -> GB2312 -> GBK -> UCS4				      \
									      \
	   might not produce identical text.  */			      \
	if (NEED_LENGTH_TEST && inptr + 1 >= inend)			      \
	  {								      \
	    /* The second character is not available.  Store		      \
	       the intermediate result.  */				      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	if (NEED_LENGTH_TEST && outend - outptr < 2)			      \
	  {								      \
	    /* We ran out of space.  */					      \
	    result = __GCONV_FULL_OUTPUT;				      \
	    break;							      \
	  }								      \
									      \
	ch = (ch << 8) | inptr[1];					      \
									      \
	/* Now determine whether the character is valid.  */		      \
	if (ch >= 0xa1a1 && ch <= 0xf7fe && inptr[1] >= 0xa1)		      \
	  {								      \
	    /* So far so good.  Now test the exceptions.  */		      \
	    if ((ch >= 0xa2a1 && ch <= 0xa2aa)				      \
		|| (ch >= 0xa6e0 && ch <= 0xa6f5)			      \
		|| (ch >= 0xa8bb && ch <= 0xa8c0))			      \
	      {								      \
		/* One of the exceptions.  */				      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	  }								      \
	else								      \
	  {								      \
	    /* One of the characters we cannot map.  */			      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	/* Copy the two bytes.  */					      \
	*outptr++ = *inptr++;						      \
	*outptr++ = *inptr++;						      \
      }									      \
  }
#include <iconv/loop.c>


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    /* We don't have to care about characters we cannot map.  The only	      \
       problem is the mapping of 0xA1A4 but as explained above we do not      \
       do anything special here.  */					      \
    unsigned char ch = *inptr++;					      \
									      \
    if (ch > 0x7f)							      \
      {									      \
	if (NEED_LENGTH_TEST && inptr + 1 >= inend)			      \
	  {								      \
	    /* The second character is not available.  Store		      \
		 the intermediate result.  */				      \
	      result = __GCONV_INCOMPLETE_INPUT;			      \
	      break;							      \
	  }								      \
									      \
	if (NEED_LENGTH_TEST && outend - outptr < 2)			      \
	  {								      \
	    /* We ran out of space.  */					      \
	    result = __GCONV_FULL_OUTPUT;				      \
	    break;							      \
	  }								      \
									      \
	*outptr++ = ch;							      \
	ch = *inptr++;							      \
      }									      \
    *outptr++ = ch;							      \
  }
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
