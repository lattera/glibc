/* Conversion to and from IBM937.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Masahide Washizawa <washi@yamato.ibm.co.jp>, 2000.

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

#include <dlfcn.h>
#include <stdint.h>
#include <wchar.h>
#include <byteswap.h>
#include "ibm937.h"

/* The shift sequences for this charset (it does not use ESC).  */
#define SI 		0x0F  /* Shift In, host code to turn DBCS off.  */
#define SO 		0x0E  /* Shift Out, host code to turn DBCS on.  */

/* Definitions used in the body of the `gconv' function.  */
#define CHARSET_NAME	"IBM937//"
#define FROM_LOOP	from_ibm937
#define TO_LOOP		to_ibm937

/* Definitions of initialization and destructor function.  */
#define DEFINE_INIT	1
#define DEFINE_FINI	1

#define MIN_NEEDED_FROM	1
#define MIN_NEEDED_TO	4

/* Current codeset type.  */
enum
{
  init = 0,
  sb,
  db
};

/* First, define the conversion function from IBM-937 to UCS4.  */
#define MIN_NEEDED_INPUT  	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT 	MIN_NEEDED_TO
#define INIT_PARAMS 		int curcs = init;
#define LOOPFCT 		FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch = *inptr;						      \
									      \
    if (__builtin_expect (ch, 0) == SO)					      \
      {									      \
	if (__builtin_expect (inptr + 1 >= inend, 0))			      \
	  {								      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	/* Shift OUT, change to DBCS converter.  */			      \
	if (curcs == db)						      \
	  {								      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
	curcs = db;							      \
	++inptr;							      \
	ch = *inptr;							      \
      }									      \
    else if (__builtin_expect (ch, 0) == SI)				      \
      {									      \
	if (__builtin_expect (inptr + 1 >= inend, 0))			      \
	  {								      \
	    result = __GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	/* Shift IN, change to SBCS converter.  */			      \
	if (curcs == sb)						      \
	  {								      \
	    result = __GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
	curcs = sb;							      \
	++inptr;							      \
	ch = *inptr;							      \
      }									      \
									      \
    if (curcs == sb || curcs == init)					      \
      {									      \
	/* Use the UCS4 table for single byte.  */			      \
	ch = __ibm937sb_to_ucs4[ch];					      \
	if (__builtin_expect (ch, L'\1') == L'\0' && *inptr != '\0')	      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	    ++inptr;							      \
	    continue;							      \
	  }								      \
	else								      \
	  {								      \
	    put32 (outptr, ch);						      \
	    outptr += 4;						      \
	    inptr++;							      \
	  }								      \
      }									      \
    else if (curcs == db)						      \
      {									      \
	/* Use the IBM937 table for double byte.  */			      \
	ch = ibm937db_to_ucs4(inptr[0], inptr[1]);			      \
	if (__builtin_expect (ch, L'\1') == L'\0' && *inptr != '\0')	      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	    inptr += 2;							      \
	    continue;							      \
	  }								      \
	else								      \
	  {								      \
	    put32 (outptr, ch);						      \
	    outptr += 4;						      \
	    inptr += 2;							      \
	  }								      \
      }									      \
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define INIT_PARAMS 		int curcs = init;
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    uint32_t ch = get32 (inptr);					      \
    const char *cp;							      \
									      \
    /* Use the UCS4 table for single byte.  */				      \
    cp = __ucs4_to_ibm937sb[ch];					      \
    if (__builtin_expect (ch >= sizeof (__ucs4_to_ibm937sb)		      \
			  / sizeof (__ucs4_to_ibm937sb[0]), 0)		      \
	|| (__builtin_expect (cp[0], '\1') == '\0' && ch != 0))		      \
      {									      \
	/* Use the UCS4 table for double byte.  */			      \
	cp = __ucs4_to_ibm937db[ch];					      \
	if (__builtin_expect (cp[0], '\1') == '\0' && ch != 0)		      \
	  {								      \
	    /* This is an illegal character.  */			      \
	    if (! ignore_errors_p ())					      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
	    ++*irreversible;						      \
	  }								      \
	else								      \
	  {								      \
	    if (curcs == init || curcs == sb)				      \
	      {								      \
		*outptr++ = SO;						      \
		if (__builtin_expect (outptr == outend, 0))		      \
		  {							      \
		    result = __GCONV_FULL_OUTPUT;			      \
		    break;						      \
		  }							      \
		curcs = db;						      \
	      }								      \
	    *outptr++ = cp[0];						      \
	    *outptr++ = cp[1];						      \
	  }								      \
      }									      \
    else								      \
      {									      \
	if (curcs == db)						      \
	  {								      \
	    *outptr++ = SI;						      \
	    if (__builtin_expect (outptr == outend, 0))			      \
	      {								      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
	  }								      \
	curcs = sb;							      \
	*outptr++ = cp[0];						      \
      }									      \
									      \
    /* Now that we wrote the output increment the input pointer.  */	      \
    inptr += 4;								      \
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
