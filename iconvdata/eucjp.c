/* Mapping tables for EUC-JP handling.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <stdint.h>
#include <gconv.h>
#include <jis0201.h>
#include <jis0208.h>
#include <jis0212.h>

/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME		"EUC-JP"
#define FROM_LOOP		from_euc_jp
#define TO_LOOP			to_euc_jp
#define DEFINE_INIT		1
#define DEFINE_FINI		1
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		3
#define MIN_NEEDED_TO		4


/* First define the conversion function from EUC-JP to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = *inptr;						      \
									      \
    if (ch <= 0x7f)							      \
      ++inptr;								      \
    else if ((ch <= 0xa0 || ch > 0xfe) && ch != 0x8e && ch != 0x8f)	      \
      {									      \
	/* This is illegal.  */						      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
    else								      \
      {									      \
	/* Two or more byte character.  First test whether the next	      \
	   character is also available.  */				      \
	int ch2;							      \
									      \
	if (NEED_LENGTH_TEST && inptr + 1 >= inend)			      \
	  {								      \
	    /* The second character is not available.  Store the	      \
	       intermediate result.  */					      \
	    result = GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	ch2 = inptr[1];							      \
									      \
	/* All second bytes of a multibyte character must be >= 0xa1. */      \
	if (ch2 < 0xa1)							      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    result = GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	if (ch == 0x8e)							      \
	  {								      \
	    /* This is code set 2: half-width katakana.  */		      \
	    ch = jisx0201_to_ucs4 (ch2);				      \
	    inptr += 2;							      \
	  }								      \
	else								      \
	  {								      \
	    const unsigned char *endp;					      \
									      \
	    if (ch == 0x8f)						      \
	      {								      \
		/* This is code set 3: JIS X 0212-1990.  */		      \
		endp = inptr + 1;					      \
									      \
		ch = jisx0212_to_ucs4 (&endp,				      \
				       NEED_LENGTH_TEST ? inend - endp : 2,   \
				       0x80);				      \
	      }								      \
	    else							      \
	      {								      \
		/* This is code set 1: JIS X 0208.  */			      \
		endp = inptr;						      \
									      \
		ch = jisx0208_to_ucs4 (&endp,				      \
				       NEED_LENGTH_TEST ? inend - inptr : 2,  \
				       0x80);				      \
	      }								      \
									      \
	    if (NEED_LENGTH_TEST && ch == 0)				      \
	      {								      \
		/* Not enough input available.  */			      \
		result = GCONV_INCOMPLETE_INPUT;			      \
		break;							      \
	      }								      \
	    if (ch == UNKNOWN_10646_CHAR)				      \
	      {								      \
		/* Illegal character.  */				      \
		result = GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    inptr = endp;						      \
	  }								      \
      }									      \
									      \
    *((uint32_t *) outptr)++ = ch;					      \
  }
#include <iconv/loop.c>


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    uint32_t ch = *((uint32_t *) inptr);				      \
									      \
    if (ch <= 0x7f)							      \
      /* It's plain ASCII.  */						      \
      *outptr++ = ch;							      \
    else								      \
      {									      \
	/* Try the JIS character sets.  */				      \
	size_t found;							      \
									      \
	/* See whether we have room for at least two characters.  */	      \
	if (NEED_LENGTH_TEST && outptr + 1 >= outend)			      \
	  {								      \
	    result = GCONV_FULL_OUTPUT;					      \
	    break;							      \
	  }								      \
									      \
	found = ucs4_to_jisx0201 (ch, outptr + 1);			      \
	if (found != UNKNOWN_10646_CHAR)				      \
	  {								      \
	    /* Yes, it's a JIS 0201 character.  Store the shift byte.  */     \
	    *outptr = 0x8e;						      \
	    outptr += 2;						      \
	  }								      \
	else								      \
	  {								      \
	    /* No JIS 0201 character.  */				      \
	    found = ucs4_to_jisx0208 (ch, outptr, 2);			      \
	    /* Please note that we always have enough room for the output. */ \
	    if (found != UNKNOWN_10646_CHAR)				      \
	      {								      \
		/* It's a JIS 0208 character, adjust it for EUC-JP.  */	      \
		*outptr++ += 0x80;					      \
		*outptr++ += 0x80;					      \
	      }								      \
	    else							      \
	      {								      \
		/* No JIS 0208 character.  */				      \
		found = ucs4_to_jisx0212 (ch, outptr + 1,		      \
					  (NEED_LENGTH_TEST		      \
					   ? outend - outptr - 1 : 2));	      \
		  							      \
		if (found == 0)						      \
		  {							      \
		    /* We ran out of space.  */				      \
		    result = GCONV_FULL_OUTPUT;				      \
		    break;						      \
		  }							      \
		else if (found != UNKNOWN_10646_CHAR)			      \
		  {							      \
		    /* It's a JIS 0212 character, adjust it for EUC-JP.  */   \
		    *outptr++ = 0x8f;					      \
		    *outptr++ += 0x80;					      \
		    *outptr++ += 0x80;					      \
		  }							      \
		else							      \
		  {							      \
		    /* Illegal character.  */				      \
		    result = GCONV_ILLEGAL_INPUT;			      \
		    break;						      \
		  }							      \
	      }								      \
	  }								      \
      }									      \
									      \
    inptr += 4;								      \
  }
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
