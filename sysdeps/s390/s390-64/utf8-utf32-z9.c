/* Conversion between UTF-8 and UTF-32 BE/internal.

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

/* UTF-32 big endian byte order mark.  */
#define BOM	                0x0000feffu

#define DEFINE_INIT		0
#define DEFINE_FINI		0
/* These definitions apply to the UTF-8 to UTF-32 direction.  The
   software implementation for UTF-8 still supports multibyte
   characters up to 6 bytes whereas the hardware variant does not.  */
#define MIN_NEEDED_FROM		1
#define MAX_NEEDED_FROM		6
#define MIN_NEEDED_TO		4
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
      /* Emit the Byte Order Mark.  */					\
      if (__glibc_unlikely (outbuf + 4 > outend))			      \
	return __GCONV_FULL_OUTPUT;					\
									\
      put32u (outbuf, BOM);						\
      outbuf += 4;							\
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

  emit_bom = (__strcasecmp (step->__to_name, "UTF-32//") == 0);

  if (__strcasecmp (step->__from_name, "ISO-10646/UTF8/") == 0
      && (__strcasecmp (step->__to_name, "UTF-32//") == 0
	  || __strcasecmp (step->__to_name, "UTF-32BE//") == 0
	  || __strcasecmp (step->__to_name, "INTERNAL") == 0))
    {
      dir = from_utf8;
    }
  else if (__strcasecmp (step->__to_name, "ISO-10646/UTF8/") == 0
	   && (__strcasecmp (step->__from_name, "UTF-32BE//") == 0
	       || __strcasecmp (step->__from_name, "INTERNAL") == 0))
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
		  "   ipm    %2        \n"				\
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

/* Conversion function from UTF-8 to UTF-32 internal/BE.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
/* The software routine is copied from gconv_simple.c.  */
#define BODY								\
  {									\
    if (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH)				\
      {									\
	HARDWARE_CONVERT ("cu14 %0, %1, 1");				\
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
    uint32_t ch = *inptr;						\
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
	    /* We expect two bytes.  The first byte cannot be 0xc0 or	\
	       0xc1, otherwise the wide character could have been	\
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
	else if (__glibc_likely ((ch & 0xfc) == 0xf8))			      \
	  {								\
	    /* We expect five bytes.  */				\
	    cnt = 5;							\
	    ch &= 0x03;							\
	  }								\
	else if (__glibc_likely ((ch & 0xfe) == 0xfc))			      \
	  {								\
	    /* We expect six bytes.  */					\
	    cnt = 6;							\
	    ch &= 0x01;							\
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
	/* Read the possible remaining bytes.  */			\
	for (i = 1; i < cnt; ++i)					\
	  {								\
	    uint32_t byte = inptr[i];					\
									\
	    if ((byte & 0xc0) != 0x80)					\
	      /* This is an illegal encoding.  */			\
	      break;							\
									\
	    ch <<= 6;							\
	    ch |= byte & 0x3f;						\
	  }								\
									\
	/* If i < cnt, some trail byte was not >= 0x80, < 0xc0.		\
	   If cnt > 2 and ch < 2^(5*cnt-4), the wide character ch could	\
	   have been represented with fewer than cnt bytes.  */		\
	if (i < cnt || (cnt > 2 && (ch >> (5 * cnt - 4)) == 0))		\
	  {								\
	    /* This is an illegal encoding.  */				\
	    goto errout;						\
	  }								\
									\
	inptr += cnt;							\
      }									\
									\
    /* Now adjust the pointers and store the result.  */		\
    *((uint32_t *) outptr) = ch;					\
    outptr += sizeof (uint32_t);					\
  }
#define LOOP_NEED_FLAGS

#define STORE_REST							\
  {									      \
    /* We store the remaining bytes while converting them into the UCS4	      \
       format.  We can assume that the first byte in the buffer is	      \
       correct and that it requires a larger number of bytes than there	      \
       are in the input buffer.  */					      \
    wint_t ch = **inptrp;						      \
    size_t cnt, r;							      \
									      \
    state->__count = inend - *inptrp;					      \
									      \
    if (ch >= 0xc2 && ch < 0xe0)					      \
      {									      \
	/* We expect two bytes.  The first byte cannot be 0xc0 or	      \
	   0xc1, otherwise the wide character could have been		      \
	   represented using a single byte.  */				      \
	cnt = 2;							      \
	ch &= 0x1f;							      \
      }									      \
    else if (__glibc_likely ((ch & 0xf0) == 0xe0))			      \
      {									      \
	/* We expect three bytes.  */					      \
	cnt = 3;							      \
	ch &= 0x0f;							      \
      }									      \
    else if (__glibc_likely ((ch & 0xf8) == 0xf0))			      \
      {									      \
	/* We expect four bytes.  */					      \
	cnt = 4;							      \
	ch &= 0x07;							      \
      }									      \
    else if (__glibc_likely ((ch & 0xfc) == 0xf8))			      \
      {									      \
	/* We expect five bytes.  */					      \
	cnt = 5;							      \
	ch &= 0x03;							      \
      }									      \
    else								      \
      {									      \
	/* We expect six bytes.  */					      \
	cnt = 6;							      \
	ch &= 0x01;							      \
      }									      \
									      \
    /* The first byte is already consumed.  */				      \
    r = cnt - 1;							      \
    while (++(*inptrp) < inend)						      \
      {									      \
	ch <<= 6;							      \
	ch |= **inptrp & 0x3f;						      \
	--r;								      \
      }									      \
									      \
    /* Shift for the so far missing bytes.  */				      \
    ch <<= r * 6;							      \
									      \
    /* Store the number of bytes expected for the entire sequence.  */	      \
    state->__count |= cnt << 8;						      \
									      \
    /* Store the value.  */						      \
    state->__value.__wch = ch;						      \
  }

#define UNPACK_BYTES \
  {									      \
    static const unsigned char inmask[5] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };  \
    wint_t wch = state->__value.__wch;					      \
    size_t ntotal = state->__count >> 8;				      \
									      \
    inlen = state->__count & 255;					      \
									      \
    bytebuf[0] = inmask[ntotal - 2];					      \
									      \
    do									      \
      {									      \
	if (--ntotal < inlen)						      \
	  bytebuf[ntotal] = 0x80 | (wch & 0x3f);			      \
	wch >>= 6;							      \
      }									      \
    while (ntotal > 1);							      \
									      \
    bytebuf[0] |= wch;							      \
  }

#define CLEAR_STATE \
  state->__count = 0

#include <iconv/loop.c>

/* Conversion from UTF-32 internal/BE to UTF-8.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
/* The software routine mimics the S/390 cu41 instruction.  */
#define BODY							\
  {								\
    if (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH)			\
      {								\
	HARDWARE_CONVERT ("cu41 %0, %1");			\
								\
	if (inptr != inend)					\
	  {							\
	    result = __GCONV_INCOMPLETE_INPUT;			\
	    break;						\
	  }							\
	continue;						\
      }								\
								\
    uint32_t wc = *((const uint32_t *) inptr);			\
								\
    if (__glibc_likely (wc <= 0x7f))					      \
      {								\
        /* Single UTF-8 char.  */				\
        *outptr = (uint8_t)wc;					\
	outptr++;						\
      }								\
    else if (wc <= 0x7ff)					\
      {								\
        /* Two UTF-8 chars.  */					\
        if (__glibc_unlikely (outptr + 2 > outend))			      \
	  {							\
	    /* Overflow in the output buffer.  */		\
	    result = __GCONV_FULL_OUTPUT;			\
	    break;						\
	  }							\
								\
        outptr[0] = 0xc0;					\
	outptr[0] |= wc >> 6;					\
								\
	outptr[1] = 0x80;					\
	outptr[1] |= wc & 0x3f;					\
								\
	outptr += 2;						\
      }								\
    else if (wc <= 0xffff)					\
      {								\
	/* Three UTF-8 chars.  */				\
	if (__glibc_unlikely (outptr + 3 > outend))			      \
	  {							\
	    /* Overflow in the output buffer.  */		\
	    result = __GCONV_FULL_OUTPUT;			\
	    break;						\
	  }							\
	outptr[0] = 0xe0;					\
	outptr[0] |= wc >> 12;					\
								\
	outptr[1] = 0x80;					\
	outptr[1] |= (wc >> 6) & 0x3f;				\
								\
	outptr[2] = 0x80;					\
	outptr[2] |= wc & 0x3f;					\
								\
	outptr += 3;						\
      }								\
      else if (wc <= 0x10ffff)					\
	{							\
	  /* Four UTF-8 chars.  */				\
	  if (__glibc_unlikely (outptr + 4 > outend))			      \
	    {							\
	      /* Overflow in the output buffer.  */		\
	      result = __GCONV_FULL_OUTPUT;			\
	      break;						\
	    }							\
	  outptr[0] = 0xf0;					\
	  outptr[0] |= wc >> 18;				\
								\
	  outptr[1] = 0x80;					\
	  outptr[1] |= (wc >> 12) & 0x3f;			\
								\
	  outptr[2] = 0x80;					\
	  outptr[2] |= (wc >> 6) & 0x3f;			\
								\
	  outptr[3] = 0x80;					\
	  outptr[3] |= wc & 0x3f;				\
								\
	  outptr += 4;						\
	}							\
      else							\
	{							\
	  STANDARD_TO_LOOP_ERR_HANDLER (4);			\
	}							\
    inptr += 4;							\
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

#include <iconv/skeleton.c>
