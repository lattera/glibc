/* Conversion module for UTF-16.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <byteswap.h>
#include <gconv.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* This is the Byte Order Mark character (BOM).  */
#define BOM	0xfeff


/* Definitions used in the body of the `gconv' function.  */
#define FROM_LOOP		from_utf16_loop
#define TO_LOOP			to_utf16_loop
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		2
#define MAX_NEEDED_FROM		4
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		(dir == from_utf16)
#define PREPARE_LOOP \
  enum direction dir = ((struct utf16_data *) step->__data)->dir;	      \
  enum variant var = ((struct utf16_data *) step->__data)->var;		      \
  if (!FROM_DIRECTION && var == UTF_16 && !data->__internal_use		      \
      && data->__invocation_counter == 0)				      \
    {									      \
      /* Emit the Byte Order Mark.  */					      \
      if (outbuf + 2 > outend)						      \
	return __GCONV_FULL_OUTPUT;					      \
									      \
      *(uint16_t *) outbuf = BOM;					      \
      outbuf += 2;							      \
    }
#define EXTRA_LOOP_ARGS		, var, data


/* Direction of the transformation.  */
enum direction
{
  illegal_dir,
  to_utf16,
  from_utf16
};

enum variant
{
  illegal_var,
  UTF_16,
  UTF_16LE,
  UTF_16BE
};

struct utf16_data
{
  enum direction dir;
  enum variant var;
};


int
gconv_init (struct __gconv_step *step)
{
  /* Determine which direction.  */
  struct utf16_data *new_data;
  enum direction dir = illegal_dir;
  enum variant var = illegal_var;
  int result;

  if (__strcasecmp (step->__from_name, "UTF-16") == 0)
    {
      dir = from_utf16;
      var = UTF_16;
    }
  else if (__strcasecmp (step->__to_name, "UTF-16") == 0)
    {
      dir = to_utf16;
      var = UTF_16;
    }
  else if (__strcasecmp (step->__from_name, "UTF-16BE") == 0)
    {
      dir = from_utf16;
      var = UTF_16BE;
    }
  else if (__strcasecmp (step->__to_name, "UTF-16BE") == 0)
    {
      dir = to_utf16;
      var = UTF_16BE;
    }
  else if (__strcasecmp (step->__from_name, "UTF-16LE") == 0)
    {
      dir = from_utf16;
      var = UTF_16LE;
    }
  else if (__strcasecmp (step->__to_name, "UTF-16LE") == 0)
    {
      dir = to_utf16;
      var = UTF_16LE;
    }

  result = __GCONV_NOCONV;
  if (dir != illegal_dir)
    {
      new_data = (struct utf16_data *) malloc (sizeof (struct utf16_data));

      result = __GCONV_NOMEM;
      if (new_data != NULL)
	{
	  new_data->dir = dir;
	  new_data->var = var;
	  step->__data = new_data;

	  if (dir == from_utf16)
	    {
	      step->__min_needed_from = MIN_NEEDED_FROM;
	      step->__max_needed_from = MIN_NEEDED_FROM;
	      step->__min_needed_to = MIN_NEEDED_TO;
	      step->__max_needed_to = MIN_NEEDED_TO;
	    }
	  else
	    {
	      step->__min_needed_from = MIN_NEEDED_TO;
	      step->__max_needed_from = MIN_NEEDED_TO;
	      step->__min_needed_to = MIN_NEEDED_FROM;
	      step->__max_needed_to = MIN_NEEDED_FROM;
	    }

	  step->__stateful = 0;

	  result = __GCONV_OK;
	}
    }

  return result;
}


void
gconv_end (struct __gconv_step *data)
{
  free (data->__data);
}


/* Convert from the internal (UCS4-like) format to UTF-16.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    uint32_t c = *((uint32_t *) inptr);					      \
									      \
    if ((__BYTE_ORDER == __LITTLE_ENDIAN && var == UTF_16BE)		      \
        || (__BYTE_ORDER == __BIG_ENDIAN && var == UTF_16LE))		      \
      {									      \
	if (c >= 0x10000)						      \
	  {								      \
	    if (c >= 0x110000)						      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    /* Generate a surrogate character.  */			      \
	    if (NEED_LENGTH_TEST && outptr + 4 > outend)		      \
	      {								      \
		/* Overflow in the output buffer.  */			      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
									      \
	    *((uint16_t *) outptr) = bswap_16 (0xd7c0 + (c >> 10));	      \
	    outptr += 2;						      \
	    *((uint16_t *) outptr) = bswap_16 (0xdc00 + (c & 0x3ff));	      \
	  }								      \
	else								      \
	  *((uint16_t *) outptr) = bswap_16 (c);			      \
      }									      \
    else								      \
      {									      \
	if (c >= 0x10000)						      \
	  {								      \
	    if (c >= 0x110000)						      \
	      {								      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    /* Generate a surrogate character.  */			      \
	    if (NEED_LENGTH_TEST && outptr + 4 > outend)		      \
	      {								      \
		/* Overflow in the output buffer.  */			      \
		result = __GCONV_FULL_OUTPUT;				      \
		break;							      \
	      }								      \
									      \
	    *((uint16_t *) outptr) = 0xd7c0 + (c >> 10);		      \
	    outptr += 2;						      \
	    *((uint16_t *) outptr) = 0xdc00 + (c & 0x3ff);		      \
	  }								      \
	else								      \
	  *((uint16_t *) outptr) = c;					      \
      }									      \
    outptr += 2;							      \
    inptr += 4;								      \
  }
#define EXTRA_LOOP_DECLS \
	, enum variant var, struct __gconv_step_data *step_data
#include <iconv/loop.c>


/* Convert from UTF-16 to the internal (UCS4-like) format.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint16_t u1 = *(uint16_t *) inptr;					      \
									      \
    if ((__BYTE_ORDER == __LITTLE_ENDIAN && var == UTF_16BE) 		      \
        || (__BYTE_ORDER == __BIG_ENDIAN && var == UTF_16LE))		      \
      {									      \
	u1 = bswap_16 (u1);						      \
									      \
	if (u1 < 0xd800 || u1 > 0xdfff)					      \
	  {								      \
	    /* No surrogate.  */					      \
	    *((uint32_t *) outptr) = u1;				      \
	    inptr += 2;							      \
	  }								      \
	else								      \
	  {								      \
	    uint16_t u2;						      \
									      \
	    /* It's a surrogate character.  At least the first word says      \
	       it is.  */						      \
	    if (NEED_LENGTH_TEST && inptr + 4 > inend)			      \
	      {								      \
		/* We don't have enough input for another complete input      \
		   character.  */					      \
		result = __GCONV_INCOMPLETE_INPUT;			      \
		break;							      \
	      }								      \
									      \
	    u2 = bswap_16 (((uint16_t *) inptr)[1]);			      \
	    if (u2 < 0xdc00 || u2 >= 0xdfff)				      \
	      {								      \
		/* This is no valid second word for a surrogate.  */	      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    *((uint32_t *) outptr) = ((u1 - 0xd7c0) << 10) + (u2 - 0xdc00);   \
	    inptr += 4;							      \
	  }								      \
      }									      \
    else								      \
      {									      \
	if (u1 == BOM && var == UTF_16 && !step_data->__internal_use	      \
	    && step_data->__invocation_counter == 0 && inptr == *inptrp)      \
	  {								      \
	    /* This is the first word in the file and it is the BOM and	      \
	       we are converting a file without specified byte order.	      \
	       Simply sack the BOM.  */					      \
	    inptr += 2;							      \
	    continue;							      \
	  }								      \
									      \
	if (u1 < 0xd800 || u1 > 0xdfff)					      \
	  {								      \
	    /* No surrogate.  */					      \
	    *((uint32_t *) outptr) = u1;				      \
	    inptr += 2;							      \
	  }								      \
	else								      \
	  {								      \
	    uint16_t u2;						      \
									      \
	    /* It's a surrogate character.  At least the first word says      \
	       it is.  */						      \
	    if (NEED_LENGTH_TEST && inptr + 4 > inend)			      \
	      {								      \
		/* We don't have enough input for another complete input      \
		   character.  */					      \
		result = __GCONV_INCOMPLETE_INPUT;			      \
		break;							      \
	      }								      \
									      \
	    u2 = ((uint16_t *) inptr)[1];				      \
	    if (u2 < 0xdc00 || u2 >= 0xdfff)				      \
	      {								      \
		/* This is no valid second word for a surrogate.  */	      \
		result = __GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    *((uint32_t *) outptr) = ((u1 - 0xd7c0) << 10) + (u2 - 0xdc00);   \
	    inptr += 4;							      \
	  }								      \
      }									      \
    outptr += 4;							      \
  }
#define EXTRA_LOOP_DECLS \
	, enum variant var, struct __gconv_step_data *step_data
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
