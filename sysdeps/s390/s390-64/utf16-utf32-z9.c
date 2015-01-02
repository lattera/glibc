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

/* UTF-32 big endian byte order mark.  */
#define BOM_UTF32               0x0000feffu

/* UTF-16 big endian byte order mark.  */
#define BOM_UTF16	        0xfeff

#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		2
#define MAX_NEEDED_FROM		4
#define MIN_NEEDED_TO		4
#define FROM_LOOP		from_utf16_loop
#define TO_LOOP			to_utf16_loop
#define FROM_DIRECTION		(dir == from_utf16)
#define ONE_DIRECTION           0
#define PREPARE_LOOP							\
  enum direction dir = ((struct utf16_data *) step->__data)->dir;	\
  int emit_bom = ((struct utf16_data *) step->__data)->emit_bom;	\
									\
  if (emit_bom && !data->__internal_use					\
      && data->__invocation_counter == 0)				\
    {									\
      if (dir == to_utf16)						\
	{								\
          /* Emit the UTF-16 Byte Order Mark.  */			\
          if (__glibc_unlikely (outbuf + 2 > outend))			      \
	    return __GCONV_FULL_OUTPUT;					\
									\
	  put16u (outbuf, BOM_UTF16);					\
	  outbuf += 2;							\
	}								\
      else								\
	{								\
          /* Emit the UTF-32 Byte Order Mark.  */			\
	  if (__glibc_unlikely (outbuf + 4 > outend))			      \
	    return __GCONV_FULL_OUTPUT;					\
									\
	  put32u (outbuf, BOM_UTF32);					\
	  outbuf += 4;							\
	}								\
    }

/* Direction of the transformation.  */
enum direction
{
  illegal_dir,
  to_utf16,
  from_utf16
};

struct utf16_data
{
  enum direction dir;
  int emit_bom;
};


extern int gconv_init (struct __gconv_step *step);
int
gconv_init (struct __gconv_step *step)
{
  /* Determine which direction.  */
  struct utf16_data *new_data;
  enum direction dir = illegal_dir;
  int emit_bom;
  int result;

  emit_bom = (__strcasecmp (step->__to_name, "UTF-32//") == 0
	      || __strcasecmp (step->__to_name, "UTF-16//") == 0);

  if (__strcasecmp (step->__from_name, "UTF-16BE//") == 0
      && (__strcasecmp (step->__to_name, "UTF-32//") == 0
	  || __strcasecmp (step->__to_name, "UTF-32BE//") == 0
	  || __strcasecmp (step->__to_name, "INTERNAL") == 0))
    {
      dir = from_utf16;
    }
  else if ((__strcasecmp (step->__to_name, "UTF-16//") == 0
	    || __strcasecmp (step->__to_name, "UTF-16BE//") == 0)
	   && (__strcasecmp (step->__from_name, "UTF-32BE//") == 0
	       || __strcasecmp (step->__from_name, "INTERNAL") == 0))
    {
      dir = to_utf16;
    }

  result = __GCONV_NOCONV;
  if (dir != illegal_dir)
    {
      new_data = (struct utf16_data *) malloc (sizeof (struct utf16_data));

      result = __GCONV_NOMEM;
      if (new_data != NULL)
	{
	  new_data->dir = dir;
	  new_data->emit_bom = emit_bom;
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

/* Conversion function from UTF-16 to UTF-32 internal/BE.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_INPUT	MAX_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
/* The software routine is copied from utf-16.c (minus bytes
   swapping).  */
#define BODY								\
  {									\
    /* The hardware instruction currently fails to report an error for	\
       isolated low surrogates so we have to disable the instruction	\
       until this gets resolved.  */					\
    if (0) /* (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH) */			\
      {									\
	HARDWARE_CONVERT ("cu24 %0, %1, 1");				\
	if (inptr != inend)						\
	  {								\
	    /* Check if the third byte is				\
	       a valid start of a UTF-16 surrogate.  */			\
	    if (inend - inptr == 3 && (inptr[3] & 0xfc) != 0xdc)	\
	      STANDARD_FROM_LOOP_ERR_HANDLER (3);			\
									\
	    result = __GCONV_INCOMPLETE_INPUT;				\
	    break;							\
	  }								\
	continue;							\
      }									\
									\
    uint16_t u1 = get16 (inptr);					\
									\
    if (__builtin_expect (u1 < 0xd800, 1) || u1 > 0xdfff)		\
      {									\
	/* No surrogate.  */						\
	put32 (outptr, u1);						\
	inptr += 2;							\
      }									\
    else								\
      {									\
        /* An isolated low-surrogate was found.  This has to be         \
	   considered ill-formed.  */					\
        if (__glibc_unlikely (u1 >= 0xdc00))				      \
	  {								\
	    STANDARD_FROM_LOOP_ERR_HANDLER (2);				\
	  }								\
	/* It's a surrogate character.  At least the first word says	\
	   it is.  */							\
	if (__glibc_unlikely (inptr + 4 > inend))			      \
	  {								\
	    /* We don't have enough input for another complete input	\
	       character.  */						\
	    result = __GCONV_INCOMPLETE_INPUT;				\
	    break;							\
	  }								\
									\
	inptr += 2;							\
	uint16_t u2 = get16 (inptr);					\
	if (__builtin_expect (u2 < 0xdc00, 0)				\
	    || __builtin_expect (u2 > 0xdfff, 0))			\
	  {								\
	    /* This is no valid second word for a surrogate.  */	\
	    inptr -= 2;							\
	    STANDARD_FROM_LOOP_ERR_HANDLER (2);				\
	  }								\
									\
	put32 (outptr, ((u1 - 0xd7c0) << 10) + (u2 - 0xdc00));		\
	inptr += 2;							\
      }									\
    outptr += 4;							\
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

/* Conversion from UTF-32 internal/BE to UTF-16.  */

#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define MAX_NEEDED_OUTPUT	MAX_NEEDED_FROM
#define LOOPFCT			TO_LOOP
/* The software routine is copied from utf-16.c (minus bytes
   swapping).  */
#define BODY								\
  {									\
    if (GLRO (dl_hwcap) & HWCAP_S390_ETF3EH)				\
      {									\
	HARDWARE_CONVERT ("cu42 %0, %1");				\
									\
	if (inptr != inend)						\
	  {								\
	    result = __GCONV_INCOMPLETE_INPUT;				\
	    break;							\
	  }								\
	continue;							\
      }									\
									\
    uint32_t c = get32 (inptr);						\
									\
    if (__builtin_expect (c <= 0xd7ff, 1)				\
	|| (c >=0xdc00 && c <= 0xffff))					\
      {									\
        /* Two UTF-16 chars.  */					\
        put16 (outptr, c);						\
      }									\
    else if (__builtin_expect (c >= 0x10000, 1)				\
	     && __builtin_expect (c <= 0x10ffff, 1))			\
      {									\
	/* Four UTF-16 chars.  */					\
        uint16_t zabcd = ((c & 0x1f0000) >> 16) - 1;			\
	uint16_t out;							\
									\
	/* Generate a surrogate character.  */				\
	if (__glibc_unlikely (outptr + 4 > outend))			      \
	  {								\
	    /* Overflow in the output buffer.  */			\
	    result = __GCONV_FULL_OUTPUT;				\
	    break;							\
	  }								\
									\
	out = 0xd800;							\
	out |= (zabcd & 0xff) << 6;					\
	out |= (c >> 10) & 0x3f;					\
	put16 (outptr, out);						\
	outptr += 2;							\
									\
	out = 0xdc00;							\
	out |= c & 0x3ff;						\
	put16 (outptr, out);						\
      }									\
    else								\
      {									\
        STANDARD_TO_LOOP_ERR_HANDLER (4);				\
      }									\
    outptr += 2;							\
    inptr += 4;								\
  }
#define LOOP_NEED_FLAGS
#include <iconv/loop.c>

#include <iconv/skeleton.c>
