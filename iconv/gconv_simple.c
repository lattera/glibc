/* Simple transformations functions.
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

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <gconv.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <sys/param.h>

#ifndef EILSEQ
# define EILSEQ EINVAL
#endif


/* These are definitions used by some of the functions for handling
   UTF-8 encoding below.  */
static const uint32_t encoding_mask[] =
{
  ~0x7ff, ~0xffff, ~0x1fffff, ~0x3ffffff
};

static const unsigned char encoding_byte[] =
{
  0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};



int
__gconv_transform_dummy (struct gconv_step *step, struct gconv_step_data *data,
			 const char **inbuf, const char *inbufend,
			 size_t *written, int do_flush)
{
  size_t do_write;

  /* We have no stateful encoding.  So we don't have to do anything
     special.  */
  if (do_flush)
    do_write = 0;
  else
    {
      do_write = MIN (inbufend - *inbuf, data->outbufend - data->outbuf);

      memcpy (data->outbuf, inbuf, do_write);

      *inbuf -= do_write;
      *data->outbuf += do_write;
    }

  /* ### TODO Actually, this number must be devided according to the
     size of the input charset.  I.e., if the input is in UCS4 the
     number of copied bytes must be divided by 4.  */
  if (written != NULL)
    *written = do_write;

  return GCONV_OK;
}


/* Transform from the internal, UCS4-like format, to UCS4.  The
   difference between the internal ucs4 format and the real UCS4
   format is, if any, the endianess.  The Unicode/ISO 10646 says that
   unless some higher protocol specifies it differently, the byte
   order is big endian.*/
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		4
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		1
#define FROM_LOOP		internal_ucs4_loop
#define TO_LOOP			internal_ucs4_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_internal_ucs4


static inline int
internal_ucs4_loop (const unsigned char **inptrp, const unsigned char *inend,
		    unsigned char **outptrp, unsigned char *outend,
		    mbstate_t *state, void *data, size_t *converted)
{
  const unsigned char *inptr = *inptrp;
  unsigned char *outptr = *outptrp;
  size_t n_convert = MIN (inend - inptr, outend - outptr) / 4;
  int result;

#if __BYTE_ORDER == __LITTLE_ENDIAN
  /* Sigh, we have to do some real work.  */
  size_t cnt;

  for (cnt = 0; cnt < n_convert; ++cnt)
    *((uint32_t *) outptr)++ = bswap_32 (*((uint32_t *) inptr)++);

  *inptrp = inptr;
  *outptrp = outptr;
#elif __BYTE_ORDER == __BIG_ENDIAN
  /* Simply copy the data.  */
  *inptrp = inptr + n_convert * 4;
  *outptrp = __mempcpy (outptr, inptr, n_convert * 4);
#else
# error "This endianess is not supported."
#endif

  /* Determine the status.  */
  if (*outptrp == outend)
    result = GCONV_FULL_OUTPUT;
  else if (*inptrp == inend)
    result = GCONV_EMPTY_INPUT;
  else
    result = GCONV_INCOMPLETE_INPUT;

  if (converted != NULL)
    converted += n_convert;

  return result;
}

#include <iconv/skeleton.c>


/* Convert from ISO 646-IRV to the internal (UCS4-like) format.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		1
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		1
#define FROM_LOOP		ascii_internal_loop
#define TO_LOOP			ascii_internal_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_ascii_internal

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    if (*inptr > '\x7f')						      \
      {									      \
	/* This is no correct ANSI_X3.4-1968 character.  */		      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
									      \
    /* It's an one byte sequence.  */					      \
    *((uint32_t *) outptr)++ = *inptr++;				      \
  }
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from the internal (UCS4-like) format to ISO 646-IRV.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		4
#define MIN_NEEDED_TO		1
#define FROM_DIRECTION		1
#define FROM_LOOP		internal_ascii_loop
#define TO_LOOP			internal_ascii_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_internal_ascii

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    if (*((uint32_t *) inptr) > 0x7f)					      \
      {									      \
	/* This is no correct ANSI_X3.4-1968 character.  */		      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
									      \
    /* It's an one byte sequence.  */					      \
    *outptr++ = *((uint32_t *) inptr)++;				      \
  }
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from the internal (UCS4-like) format to UTF-8.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		4
#define MIN_NEEDED_TO		1
#define MAX_NEEDED_TO		6
#define FROM_DIRECTION		1
#define FROM_LOOP		internal_utf8_loop
#define TO_LOOP			internal_utf8_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_internal_utf8

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t wc = *((uint32_t *) inptr);				      \
									      \
    /* Since we control every character we read this cannot happen.  */	      \
    assert (wc <= 0x7fffffff);						      \
									      \
    if (wc < 0x80)							      \
      /* It's an one byte sequence.  */					      \
      *outptr++ = (unsigned char) wc;					      \
    else								      \
      {									      \
	size_t step;							      \
	char *start;							      \
									      \
	for (step = 2; step < 6; ++step)				      \
	  if ((wc & encoding_mask[step - 2]) == 0)			      \
	    break;							      \
									      \
	if (outptr + step >= outend)					      \
	  {								      \
	    /* Too long.  */						      \
	    result = GCONV_FULL_OUTPUT;					      \
	    break;							      \
	  }								      \
									      \
	start = outptr;							      \
	*outptr = encoding_byte[step - 2];				      \
	outptr += step;							      \
	--step;								      \
	do								      \
	  {								      \
	    start[step] = 0x80 | (wc & 0x3f);				      \
	    wc >>= 6;							      \
	  }								      \
	while (--step > 0);						      \
	start[0] |= wc;							      \
      }									      \
									      \
    inptr += 4;								      \
  }
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from UTF-8 to the internal (UCS4-like) format.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		6
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		1
#define FROM_LOOP		utf8_internal_loop
#define TO_LOOP			utf8_internal_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_utf8_internal

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch;							      \
    uint_fast32_t cnt;							      \
    uint_fast32_t i;							      \
									      \
    /* Next input byte.  */						      \
    ch = *inptr;							      \
									      \
    if (ch < 0x80)							      \
      {									      \
	/* One byte sequence.  */					      \
	cnt = 1;							      \
	++inptr;							      \
      }									      \
    else								      \
      {									      \
	if ((ch & 0xe0) == 0xc0)					      \
	  {								      \
	    cnt = 2;							      \
	    ch &= 0x1f;							      \
	  }								      \
        else if ((ch & 0xf0) == 0xe0)					      \
	  {								      \
	    /* We expect three bytes.  */				      \
	    cnt = 3;							      \
	    ch &= 0x0f;							      \
	  }								      \
	else if ((ch & 0xf8) == 0xf0)					      \
	  {								      \
	    /* We expect four bytes.  */				      \
	    cnt = 4;							      \
	    ch &= 0x07;							      \
	  }								      \
	else if ((ch & 0xfc) == 0xf8)					      \
	  {								      \
	    /* We expect five bytes.  */				      \
	    cnt = 5;							      \
	    ch &= 0x03;							      \
	  }								      \
	else if ((ch & 0xfe) == 0xfc)					      \
	  {								      \
	    /* We expect six bytes.  */					      \
	    cnt = 6;							      \
	    ch &= 0x01;							      \
	  }								      \
	else								      \
	  {								      \
	    /* This is an illegal encoding.  */				      \
	    result = GCONV_ILLEGAL_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	if (NEED_LENGTH_TEST && inptr + cnt > inend)			      \
	  {								      \
	    /* We don't have enough input.  */				      \
	    result = GCONV_INCOMPLETE_INPUT;				      \
	    break;							      \
	  }								      \
									      \
	/* Read the possible remaining bytes.  */			      \
	for (i = 1; i < cnt; ++i)					      \
	  {								      \
	    uint32_t byte = inptr[i];					      \
									      \
	    if ((byte & 0xc0) != 0x80)					      \
	      {								      \
		/* This is an illegal encoding.  */			      \
		result = GCONV_ILLEGAL_INPUT;				      \
		break;							      \
	      }								      \
									      \
	    ch <<= 6;							      \
	    ch |= byte & 0x3f;						      \
	  }								      \
	inptr += cnt;							      \
      }									      \
									      \
    /* Now adjust the pointers and store the result.  */		      \
    *((uint32_t *) outptr)++ = ch;					      \
  }
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from UCS2 to the internal (UCS4-like) format.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		2
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		1
#define FROM_LOOP		ucs2_internal_loop
#define TO_LOOP			ucs2_internal_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_ucs2_internal

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define BODY \
  *((uint32_t *) outptr)++ = bswap_16 (*((uint16_t *) inptr)++);
#else
# define BODY \
  *((uint32_t *) outptr)++ = *((uint16_t *) inptr)++;
#endif
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from the internal (UCS4-like) format to UCS2.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		4
#define MIN_NEEDED_TO		2
#define FROM_DIRECTION		1
#define FROM_LOOP		internal_ucs2_loop
#define TO_LOOP			internal_ucs2_loop /* This is not used.  */
#define FUNCTION_NAME		__gconv_transform_internal_ucs2

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define BODY \
  {									      \
    if (*((uint32_t *) inptr) >= 0x10000)				      \
      {									      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
    /* Please note that we use the `uint32_t' from-pointer as an `uint16_t'   \
       pointer which works since we are on a little endian machine.  */	      \
    *((uint16_t *) outptr)++ = bswap_16 (*((uint16_t *) inptr));	      \
    inptr += 4;								      \
  }
#else
# define BODY \
  {									      \
    if (*((uint32_t *) inptr) >= 0x10000)				      \
      {									      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
    *((uint16_t *) outptr)++ = *((uint32_t *) inptr)++;			      \
  }
#endif
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from UCS2 in little endian to the internal (UCS4-like) format.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		2
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		1
#define FROM_LOOP		ucs2little_internal_loop
#define TO_LOOP			ucs2little_internal_loop /* This is not used.*/
#define FUNCTION_NAME		__gconv_transform_ucs2little_internal

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define BODY \
  *((uint32_t *) outptr)++ = *((uint16_t *) inptr)++;
#else
# define BODY \
  *((uint32_t *) outptr)++ = bswap_16 (*((uint16_t *) inptr)++);
#endif
#include <iconv/loop.c>
#include <iconv/skeleton.c>


/* Convert from the internal (UCS4-like) format to UCS2 in little endian.  */
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		4
#define MIN_NEEDED_TO		2
#define FROM_DIRECTION		1
#define FROM_LOOP		internal_ucs2little_loop
#define TO_LOOP			internal_ucs2little_loop /* This is not used.*/
#define FUNCTION_NAME		__gconv_transform_internal_ucs2little

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#if __BYTE_ORDER == __LITTLE_ENDIAN
# define BODY \
  {									      \
    if (*((uint32_t *) inptr) >= 0x10000)				      \
      {									      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
    *((uint16_t *) outptr)++ = *((uint32_t *) inptr)++;			      \
  }
#else
# define BODY \
  {									      \
    if (*((uint32_t *) inptr) >= 0x10000)				      \
      {									      \
	result = GCONV_ILLEGAL_INPUT;					      \
	break;								      \
      }									      \
    /* Please note that we use the `uint32_t' from-pointer as an `uint16_t'   \
       pointer which works since we are on a little endian machine.  */	      \
    *((uint16_t *) outptr)++ = bswap_16 (*((uint16_t *) inptr));	      \
    inptr += 4;								      \
  }
#endif
#include <iconv/loop.c>
#include <iconv/skeleton.c>
