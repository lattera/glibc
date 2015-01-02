/* Conversion between UTF-16 and UTF-32 BE/internal.

   This module uses the Z9-109 variants of the Convert Unicode
   instructions.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.

   Author: Andreas Krebbel  <Andreas.Krebbel@de.ibm.com>
   Based on the work by Ulrich Drepper  <drepper@cygnus.com>, 1997.

   Thanks to Daniel Appich who covered the relevant performance work
   in his diploma thesis.

   This is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdint.h>
#include <unistd.h>
#include <dl-procinfo.h>
#include <gconv.h>

/* UTF-16 big endian byte order mark.  */
#define BOM_UTF16	0xfeff

#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		4
#define MIN_NEEDED_TO		2
#define MAX_NEEDED_TO		4
#define FROM_LOOP		from_utf8_loop
#define TO_LOOP			to_utf8_loop
#define FROM_DIRECTION		(dir == from_utf8)
#define ONE_DIRECTION           0
#define PREPARE_LOOP							\
  enum direction dir = ((struct utf8_data *) step->__data)->dir;	\
  int emit_bom = ((struct utf8_data *) step->__data)->emit_bom;		\
									\
  if (emit_bom && !data->__internal_use					\
      && data->__invocation_counter == 0)				\
    {									\
      /* Emit the UTF-16 Byte Order Mark.  */				\
      if (__glibc_unlikely (outbuf + 2 > outend))			      \
	return __GCONV_FULL_OUTPUT;					\
									\
      put16u (outbuf, BOM_UTF16);					\
      outbuf += 2;							\
    }

/* Direction of the transformation.  */
enum direction
{
  illegal_dir,
  to_utf8,
  from_utf8
};

struct utf8_data
{
  enum direction dir;
  int emit_bom;
};


extern int gconv_init (struct __gconv_step *step);
int
gconv_init (struct __gconv_step *step)
{
  /* Determine which direction.  */
  struct utf8_data *new_data;
  enum direction dir = illegal_dir;
  int emit_bom;
  int result;

  emit_bom = (__strcasecmp (step->__to_name, "UTF-16//") == 0);

  if (__strcasecmp (step->__from_name, "ISO-10646/UTF8/") == 0
      && (__strcasecmp (step->__to_name, "UTF-16//") == 0
	  || __strcasecmp (step->__to_name, "UTF-16BE//") == 0))
    {
      dir = from_utf8;
    }
  else if (__strcasecmp (step->__from_name, "UTF-16BE//") == 0
	   && __strcasecmp (step->__to_name, "ISO-10646/UTF8/") == 0)
    {
      dir = to_utf8;
    }

  result = __GCONV_NOCONV;
  if (dir != illegal_dir)
    {
      new_data = (struct utf8_data *) malloc (sizeof (struct utf8_data));

      result = __GCONV_NOMEM;
      if (new_data != NULL)
	{
	  new_data->dir = dir;
	  new_data->emit_bom = emit_bom;
	  step->__data = new_data;

	  if (dir == from_utf8)
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


extern void gconv_end (struct __gconv_step *data);
void
gconv_end (struct __gconv_step *data)
{
  free (data->__data);
}

/* The macro for the hardware loop.  This is used for both
   directions.  */
#define HARDWARE_CONVERT(INSTRUCTION)					\
  {									\
    register const unsigned char* pInput asm ("8") = inptr;		\
    register unsigned long long inlen asm ("9") = inend - inptr;	\
    register unsigned char* pOutput asm ("10") = outptr;		\
    register unsigned long long outlen asm("11") = outend - outptr;	\
    uint64_t cc = 0;							\
									\
    asm volatile (".machine push       \n\t"				\
                  ".machine \"z9-109\" \n\t"				\
		  "0: " INSTRUCTION "  \n\t"				\
                  ".machine pop        \n\t"				\
                  "   jo     0b        \n\t"				\
		  "   ipm    %2        \n"			        \
		  : "+a" (pOutput), "+a" (pInput), "+d" (cc),		\
		    "+d" (outlen), "+d" (inlen)				\
		  :							\
		  : "cc", "memory");					\
									\
    inptr = pInput;							\
    outptr = pOutput;							\
    cc >>= 28;								\
									\
    if (cc == 1)							\
      {									\
	result = __GCONV_FULL_OUTPUT;					\
	break;								\
      }									\
    else if (cc == 2)							\
      {									\
	result = __GCONV_ILLEGAL_INPUT;					\
	break;								\
      }									\
  }

/* Conversion function from UTF-8 to UTF-16.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
/* The software implementation is based on the code in gconv_simple.c.  */
#define BODY								\
  {									\
    if (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH)				\
      {									\
	HARDWARE_CONVERT ("cu12 %0, %1, 1");				\
									\
	if (inptr != inend)						\
	  {								\
	    int i;							\
	    for (i = 1; inptr + i < inend; ++i)				\
	      if ((inptr[i] & 0xc0) != 0x80)				\
		break;							\
								\
	    if (__glibc_likely (inptr + i == inend))			      \
	      {								\
		result = __GCONV_INCOMPLETE_INPUT;			\
		break;							\
	      }								\
	    STANDARD_FROM_LOOP_ERR_HANDLER (i);				\
	  }								\
	continue;							\
    }									\
									\
    /* Next input byte.  */						\
    uint16_t ch = *inptr;						\
									\
    if (__glibc_likely (ch < 0x80))					      \
      {									\
	/* One byte sequence.  */					\
	++inptr;							\
      }									\
    else								\
      {									\
	uint_fast32_t cnt;						\
	uint_fast32_t i;						\
									\
	if (ch >= 0xc2 && ch < 0xe0)					\
	  {								\
	    /* We expect two bytes.  The first byte cannot be 0xc0	\
	       or 0xc1, otherwise the wide character could have been	\
	       represented using a single byte.  */			\
	    cnt = 2;							\
	    ch &= 0x1f;							\
	  }								\
        else if (__glibc_likely ((ch & 0xf0) == 0xe0))			      \
	  {								\
	    /* We expect three bytes.  */				\
	    cnt = 3;							\
	    ch &= 0x0f;							\
	  }								\
	else if (__glibc_likely ((ch & 0xf8) == 0xf0))			      \
	  {								\
	    /* We expect four bytes.  */				\
	    cnt = 4;							\
	    ch &= 0x07;							\
	  }								\
	else								\
	  {								\
	    /* Search the end of this ill-formed UTF-8 character.  This	\
	       is the next byte with (x & 0xc0) != 0x80.  */		\
	    i = 0;							\
	    do								\
	      ++i;							\
	    while (inptr + i < inend					\
		   && (*(inptr + i) & 0xc0) == 0x80			\
		   && i < 5);						\
									\
	  errout:							\
	    STANDARD_FROM_LOOP_ERR_HANDLER (i);				\
	  }								\
									\
	if (__glibc_unlikely (inptr + cnt > inend))			      \
	  {								\
	    /* We don't have enough input.  But before we report	\
	       that check that all the bytes are correct.  */		\
	    for (i = 1; inptr + i < inend; ++i)				\
	      if ((inptr[i] & 0xc0) != 0x80)				\
		break;							\
									\
	    if (__glibc_likely (inptr + i == inend))			      \
	      {								\
		result = __GCONV_INCOMPLETE_INPUT;			\
		break;							\
	      }								\
									\
	    goto errout;						\
	  }								\
									\
	if (cnt == 4)							\
	  {								\
	    /* For 4 byte UTF-8 chars two UTF-16 chars (high and	\
	       low) are needed.  */					\
	    uint16_t zabcd, high, low;					\
									\
	    if (__glibc_unlikely (outptr + 4 > outend))			      \
	      {								\
		/* Overflow in the output buffer.  */			\
		result = __GCONV_FULL_OUTPUT;				\
		break;							\
	      }								\
									\
	    /* See Principles of Operations cu12.  */			\
	    zabcd = (((inptr[0] & 0x7) << 2) |				\
                     ((inptr[1] & 0x30) >> 4)) - 1;			\
									\
	    /* z-bit must be zero after subtracting 1.  */		\
	    if (zabcd & 0x10)						\
	      STANDARD_FROM_LOOP_ERR_HANDLER (4)			\
									\
	    high = (uint16_t)(0xd8 << 8);       /* high surrogate id */ \
	    high |= zabcd << 6;	                        /* abcd bits */	\
	    high |= (inptr[1] & 0xf) << 2;              /* efgh bits */	\
	    high |= (inptr[2] & 0x30) >> 4;               /* ij bits */	\
									\
	    low = (uint16_t)(0xdc << 8);         /* low surrogate id */ \
	    low |= ((uint16_t)inptr[2] & 0xc) << 6;       /* kl bits */	\
	    low |= (inptr[2] & 0x3) << 6;                 /* mn bits */	\
	    low |= inptr[3] & 0x3f;                   /* opqrst bits */	\
									\
	    put16 (outptr, high);					\
	    outptr += 2;						\
	    put16 (outptr, low);					\
	    outptr += 2;						\
	    inptr += 4;							\
	    continue;							\
	  }								\
	else								\
	  {								\
	    /* Read the possible remaining bytes.  */			\
	    for (i = 1; i < cnt; ++i)					\
	      {								\
		uint16_t byte = inptr[i];				\
									\
		if ((byte & 0xc0) != 0x80)				\
		  /* This is an illegal encoding.  */			\
		  break;						\
									\
		ch <<= 6;						\
		ch |= byte & 0x3f;					\
	      }								\
	    inptr += cnt;						\
									\
	  }								\
      }									\
    /* Now adjust the pointers and store the result.  */		\
    *((uint16_t *) outptr) = ch;					\
    outptr += sizeof (uint16_t);					\
  }

#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Conversion from UTF-16 to UTF-8.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
/* The software routine is based on the functionality of the S/390
   hardware instruction (cu21) as described in the Principles of
   Operation.  */
#define BODY								\
  {									\
    /* The hardware instruction currently fails to report an error for	\
       isolated low surrogates so we have to disable the instruction	\
       until this gets resolved.  */					\
    if (0) /* (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH) */			\
      {									\
	HARDWARE_CONVERT ("cu21 %0, %1, 1");				\
	if (inptr != inend)						\
	  {								\
	    /* Check if the third byte is				\
	       a valid start of a UTF-16 surrogate.  */			\
	    if (inend - inptr == 3 && (inptr[3] & 0xfc) != 0xdc)	\
	      STANDARD_TO_LOOP_ERR_HANDLER (3);				\
									\
	    result = __GCONV_INCOMPLETE_INPUT;				\
	    break;							\
	  }								\
	continue;							\
      }									\
									\
    uint16_t c = get16 (inptr);						\
									\
    if (__glibc_likely (c <= 0x007f))					      \
      {									\
	/* Single byte UTF-8 char.  */					\
	*outptr = c & 0xff;						\
	outptr++;							\
      }									\
    else if (c >= 0x0080 && c <= 0x07ff)				\
      {									\
        /* Two byte UTF-8 char.  */					\
									\
	if (__glibc_unlikely (outptr + 2 > outend))			      \
	  {								\
	    /* Overflow in the output buffer.  */			\
	    result = __GCONV_FULL_OUTPUT;				\
	    break;							\
	  }								\
									\
        outptr[0] = 0xc0;						\
        outptr[0] |= c >> 6;						\
									\
        outptr[1] = 0x80;						\
        outptr[1] |= c & 0x3f;						\
									\
	outptr += 2;							\
      }									\
    else if ((c >= 0x0800 && c <= 0xd7ff) || c > 0xdfff)		\
      {									\
	/* Three byte UTF-8 char.  */					\
									\
	if (__glibc_unlikely (outptr + 3 > outend))			      \
	  {								\
	    /* Overflow in the output buffer.  */			\
	    result = __GCONV_FULL_OUTPUT;				\
	    break;							\
	  }								\
	outptr[0] = 0xe0;						\
	outptr[0] |= c >> 12;						\
									\
	outptr[1] = 0x80;						\
	outptr[1] |= (c >> 6) & 0x3f;					\
									\
	outptr[2] = 0x80;						\
	outptr[2] |= c & 0x3f;						\
									\
	outptr += 3;							\
      }									\
    else if (c >= 0xd800 && c <= 0xdbff)				\
      {									\
        /* Four byte UTF-8 char.  */					\
	uint16_t low, uvwxy;						\
									\
	if (__glibc_unlikely (outptr + 4 > outend))			      \
	  {								\
	    /* Overflow in the output buffer.  */			\
	    result = __GCONV_FULL_OUTPUT;				\
	    break;							\
	  }								\
	inptr += 2;							\
	if (__glibc_unlikely (inptr + 2 > inend))			      \
	  {								\
	    result = __GCONV_INCOMPLETE_INPUT;				\
	    break;							\
	  }								\
									\
	low = get16 (inptr);						\
									\
	if ((low & 0xfc00) != 0xdc00)					\
	  {								\
	    inptr -= 2;							\
	    STANDARD_TO_LOOP_ERR_HANDLER (2);				\
	  }								\
	uvwxy = ((c >> 6) & 0xf) + 1;					\
	outptr[0] = 0xf0;						\
	outptr[0] |= uvwxy >> 2;					\
									\
	outptr[1] = 0x80;						\
	outptr[1] |= (uvwxy << 4) & 0x30;				\
	outptr[1] |= (c >> 2) & 0x0f;					\
									\
	outptr[2] = 0x80;						\
	outptr[2] |= (c & 0x03) << 4;					\
	outptr[2] |= (low >> 6) & 0x0f;					\
									\
	outptr[3] = 0x80;						\
	outptr[3] |= low & 0x3f;					\
									\
	outptr += 4;							\
      }									\
    else								\
      {									\
        STANDARD_TO_LOOP_ERR_HANDLER (2);				\
      }									\
    inptr += 2;								\
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

#include <iconv/skeleton.c>
