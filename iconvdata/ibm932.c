/* Conversion from and to IBM932.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Masahide Washizawa <washi@jp.ibm.com>, 2000.

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

#include "ibm932.h"

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#define FROM	0
#define TO	1

/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME	"IBM932//"
#define FROM_LOOP	from_ibm932
#define TO_LOOP		to_ibm932

/* Definitions of initialization and destructor function.  */
#define DEFINE_INIT	1
#define DEFINE_FINI	1

#define MIN_NEEDED_FROM	1
#define MAX_NEEDED_FROM	2
#define MIN_NEEDED_TO	4

/* First, define the conversion function from IBM-932 to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    const struct gap *rp1 = __ibm932sb_to_ucs4_idx;			      \
    const struct gap *rp2 = __ibm932db_to_ucs4_idx;			      \
    uint32_t ch = *inptr;						      \
    uint32_t res;							      \
									      \
    if (__builtin_expect (ch >= 0xffff, 0))				      \
      {									      \
	rp1 = NULL;							      \
	rp2 = NULL;							      \
      }									      \
    else if (__builtin_expect (ch, 0) == 0x80				      \
	     || __builtin_expect (ch, 0) == 0xa0			      \
	     || __builtin_expect (ch, 0) == 0xfd			      \
	     || __builtin_expect (ch, 0) == 0xfe			      \
	     || __builtin_expect (ch, 0) == 0xff)			      \
      {									      \
	/* This is an illegal character.  */				      \
	if (! ignore_errors_p ())					      \
	  {								      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
      }									      \
    else								      \
      {									      \
	while (ch > rp1->end)						      \
	  ++rp1;							      \
      }									      \
									      \
    /* Use the IBM932 table for single byte.  */			      \
    if (__builtin_expect (rp1 == NULL, 0)				      \
	|| __builtin_expect (ch < rp1->start, 0)			      \
	|| (res = __ibm932sb_to_ucs4[ch + rp1->idx],			      \
	__builtin_expect (res, '\1') == 0 && ch != 0))			      \
      {									      \
									      \
	/* Use the IBM932 table for double byte.  */			      \
	if (__builtin_expect (inptr + 1 >= inend, 0))			      \
	  {								      \
	    /* The second character is not available.			      \
	       Store the intermediate result.  */			      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	ch = (ch * 0x100) + inptr[1];					      \
	while (ch > rp2->end)						      \
	  ++rp2;							      \
									      \
	if (__builtin_expect (rp2 == NULL, 0)				      \
	    || __builtin_expect (ch < rp2->start, 0)			      \
	    || (res = __ibm932db_to_ucs4[ch + rp2->idx],		      \
	    __builtin_expect (res, '\1') == 0 && ch !=0))		      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	      ++*irreversible;						      \
	      inptr += 2;						      \
	      continue;							      \
	  }								      \
	else								      \
	  {								      \
	    put32 (outptr, res);					      \
	    outptr += 4;						      \
	    inptr += 2;							      \
	  }								      \
      }									      \
    else								      \
      {									      \
	if (res == 0x1c)						      \
	  res = 0x1a;							      \
	else if (res == 0x7f)						      \
	  res = 0x1c;							      \
	else if (res == 0xa5)						      \
	  res = 0x5c;							      \
	else if (res == 0x203e)						      \
	  res = 0x7e;							      \
	else if (res == 0x1a)						      \
	  res = 0x7f;							      \
	put32 (outptr, res);						      \
	outptr += 4;							      \
	inptr++;							      \
      }									      \
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    const struct gap *rp = __ucs4_to_ibm932sb_idx;			      \
    unsigned char sc;							      \
    uint32_t ch = get32 (inptr);					      \
    uint16_t found = TRUE;						      \
    uint32_t i;								      \
    uint32_t low;							      \
    uint32_t high;							      \
    uint16_t pccode;							      \
									      \
    if (__builtin_expect (ch >= 0xffff, 0))				      \
      {									      \
	UNICODE_TAG_HANDLER (ch, 4);					      \
	rp = NULL;							      \
      }									      \
    else								      \
      while (ch > rp->end)						      \
	++rp;								      \
									      \
    /* Use the UCS4 table for single byte.  */				      \
    if (__builtin_expect (rp == NULL, 0)				      \
	|| __builtin_expect (ch < rp->start, 0)				      \
	|| (sc = __ucs4_to_ibm932sb[ch + rp->idx],			      \
	__builtin_expect (sc, '\1') == '\0' && ch != L'\0'))		      \
      {									      \
									      \
	/* Use the UCS4 table for double byte.  */			      \
	found = FALSE;							      \
	low = 0;							      \
	high = (sizeof (__ucs4_to_ibm932db) >> 1)			      \
		/ sizeof (__ucs4_to_ibm932db[0][FROM]);			      \
	pccode = ch;							      \
	while (low <= high)						      \
	  {								      \
	    i = (low + high) >> 1;					      \
	    if (pccode < __ucs4_to_ibm932db[i][FROM])			      \
	      high = i - 1;						      \
	    else if (pccode > __ucs4_to_ibm932db[i][FROM])		      \
	      low = i + 1;						      \
	    else 							      \
	      {								      \
		pccode = __ucs4_to_ibm932db[i][TO];			      \
		found = TRUE;						      \
		break;							      \
	      }								      \
	  }								      \
	if (found) 							      \
	  {								      \
	    if (__builtin_expect (outptr + 2 > outend, 0))		      \
	      {								      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
	    *outptr++ = pccode >> 8 & 0xff;				      \
	    *outptr++ = pccode & 0xff;					      \
	  }								      \
	else								      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	  }								      \
      }									      \
    else								      \
      {									      \
	if (__builtin_expect (outptr + 1 > outend, 0))			      \
	  {								      \
	    result = __GCONV_FULL_OUTPUT;				      \
	    break;							      \
	  }								      \
	if (ch == 0x5c)							      \
	  *outptr++ = 0x5c;						      \
	else if (ch == 0x7e)						      \
	  *outptr++ = 0x7e;						      \
	else								      \
	  *outptr++ = sc;						      \
      }									      \
									      \
    /* Now that we wrote the output increment the input pointer.  */	      \
    inptr += 4;								      \
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
