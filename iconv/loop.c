/* Conversion loop frame work.
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

/* This file provides a frame for the reader loop in all conversion modules.
   The actual code must (of course) be provided in the actual module source
   code but certain actions can be written down generically, with some
   customization options which are these:

     MIN_NEEDED_INPUT	minimal number of input bytes needed for the next
			conversion.
     MIN_NEEDED_OUTPUT	minimal number of bytes produced by the next round
			of conversion.

     MAX_NEEDED_INPUT	you guess it, this is the maximal number of input
			bytes needed.  It defaults to MIN_NEEDED_INPUT
     MAX_NEEDED_OUTPUT	likewise for output bytes.

     LOOPFCT		name of the function created.  If not specified
			the name is `loop' but this prevents the use
			of multiple functions in the same file.

     COUNT_CONVERTED	optional macro which is used to count the actual
			number of characters converted.  For some conversion
			it is easy to compute the value afterwards, but for
			others explicit counting is cheaper.

     BODY		this is supposed to expand to the body of the loop.
			The user must provide this.

     EXTRA_LOOP_DECLS	extra arguments passed from converion loop call.

     INIT_PARAMS	code to define and initialize variables from params.
     UPDATE_PARAMS	code to store result in params.
*/

#include <gconv.h>
#include <sys/param.h>		/* For MIN.  */
#define __need_size_t
#include <stddef.h>


/* We need at least one byte for the next round.  */
#ifndef MIN_NEEDED_INPUT
# error "MIN_NEEDED_INPUT definition missing"
#endif

/* Let's see how many bytes we produce.  */
#ifndef MAX_NEEDED_INPUT
# define MAX_NEEDED_INPUT	MIN_NEEDED_INPUT
#endif

/* We produce at least one byte in the next round.  */
#ifndef MIN_NEEDED_OUTPUT
# error "MIN_NEEDED_OUTPUT definition missing"
#endif

/* Let's see how many bytes we produce.  */
#ifndef MAX_NEEDED_OUTPUT
# define MAX_NEEDED_OUTPUT	MIN_NEEDED_OUTPUT
#endif

/* Default name for the function.  */
#ifndef LOOPFCT
# define LOOPFCT		loop
#endif

/* Make sure we have a loop body.  */
#ifndef BODY
# error "Definition of BODY missing for function" LOOPFCT
#endif

/* We can calculate the number of converted characters easily if one
   of the character sets has a fixed width.  */
#ifndef COUNT_CONVERTED
# if MIN_NEEDED_INPUT == MAX_NEEDED_INPUT
#  if MIN_NEEDED_OUTPUT == MAX_NEEDED_OUTPUT
/* Decide whether one of the charsets has size 1.  */
#   if MIN_NEEDED_INPUT == 1
#    define COUNT_CONVERTED	(inptr - *inptrp)
#   elif MIN_NEEDED_OUTPUT == 1
#    define COUNT_CONVERTED	(outptr - *outptrp)
#   else
/* Else we should see whether one of the two numbers is a power of 2.  */
#    define COUNT_CONVERTED \
  ((MIN_NEEDED_INPUT & (-MIN_NEEDED_INPUT)) == MIN_NEEDED_INPUT		      \
   ? (inptr - *inptrp) : (outptr - *outptrp))
#   endif
#  else
#   define COUNT_CONVERTED	((inptr - *inptrp) / MIN_NEEDED_INPUT)
#  endif
# elif MIN_NEEDED_OUTPUT == MAX_NEEDED_OUTPUT
#  define COUNT_CONVERTED	((outptr - *outptrp) / MIN_NEEDED_OUTPUT)
# endif
#endif


/* If no arguments have to passed to the loop function define the macro
   as empty.  */
#ifndef EXTRA_LOOP_DECLS
# define EXTRA_LOOP_DECLS
#endif


/* The function returns the status, as defined in gconv.h.  */
static inline int
LOOPFCT (const unsigned char **inptrp, const unsigned char *inend,
	 unsigned char **outptrp, unsigned char *outend, mbstate_t *state,
	 void *data, size_t *converted EXTRA_LOOP_DECLS)
{
  int result = GCONV_OK;
  const unsigned char *inptr = *inptrp;
  unsigned char *outptr = *outptrp;
#ifndef COUNT_CONVERTED
  size_t done = 0;
#endif

  /* We run one loop where we avoid checks for underflow/overflow of the
     buffers to speed up the conversion a bit.  */
  size_t min_in_rounds = (inend - inptr) / MAX_NEEDED_INPUT;
  size_t min_out_rounds = (outend - outptr) / MAX_NEEDED_OUTPUT;
  size_t min_rounds = MIN (min_in_rounds, min_out_rounds);

#ifdef INIT_PARAMS
  INIT_PARAMS;
#endif

#undef NEED_LENGTH_TEST
#define NEED_LENGTH_TEST	0
  while (min_rounds-- > 0)
    {
      /* Here comes the body the user provides.  It can stop with RESULT
	 set to GCONV_INCOMPLETE_INPUT (if the size of the input characters
	 vary in size), GCONV_ILLEGAL_INPUT, or GCONV_FULL_OUTPUT (if the
	 output characters vary in size.  */
      BODY

      /* If necessary count the successful conversion.  */
#ifndef COUNT_CONVERTED
      ++done;
#endif
    }

  if (result == GCONV_OK)
    {
#if MIN_NEEDED_INPUT == MAX_NEEDED_INPUT \
    && MIN_NEEDED_OUTPUT == MAX_NEEDED_OUTPUT
      /* We don't need to start another loop since we were able to determine
	 the maximal number of characters to copy in advance.  What remains
	 to be determined is the status.  */
      if (inptr == inend)
	/* No more input.  */
	result = GCONV_EMPTY_INPUT;
      else if ((MIN_NEEDED_OUTPUT != 1 && outptr + MIN_NEEDED_OUTPUT > outend)
	       || (MIN_NEEDED_OUTPUT == 1 && outptr >= outend))
	/* Overflow in the output buffer.  */
	result = GCONV_FULL_OUTPUT;
      else
	/* We have something left in the input buffer.  */
	result = GCONV_INCOMPLETE_INPUT;
#else
      result = GCONV_EMPTY_INPUT;

# undef NEED_LENGTH_TEST
# define NEED_LENGTH_TEST	1
      while (inptr != inend)
	{
	  /* `if' cases for MIN_NEEDED_OUTPUT ==/!= 1 is made to help the
	     compiler generating better code.  It will optimized away
	     since MIN_NEEDED_OUTPUT is always a constant.  */
	  if ((MIN_NEEDED_OUTPUT != 1 && outptr + MIN_NEEDED_OUTPUT > outend)
	      || (MIN_NEEDED_OUTPUT == 1 && outptr >= outend))
	    {
	      /* Overflow in the output buffer.  */
	      result = GCONV_FULL_OUTPUT;
	      break;
	    }
	  if (MIN_NEEDED_INPUT > 1 && inptr + MIN_NEEDED_INPUT > inend)
	    {
	      /* We don't have enough input for another complete input
		 character.  */
	      result = GCONV_INCOMPLETE_INPUT;
	      break;
	    }

	  /* Here comes the body the user provides.  It can stop with
	     RESULT set to GCONV_INCOMPLETE_INPUT (if the size of the
	     input characters vary in size), GCONV_ILLEGAL_INPUT, or
	     GCONV_FULL_OUTPUT (if the output characters vary in size).  */
	  BODY

	  /* If necessary count the successful conversion.  */
# ifndef COUNT_CONVERTED
	  ++done;
# endif
	}
#endif	/* Input and output charset are not both fixed width.  */
    }

  /* Add the number of characters we actually converted.  */
#ifdef COUNT_CONVERTED
  *converted += COUNT_CONVERTED;
#else
  *converted += done;
#endif

  /* Update the pointers pointed to by the parameters.  */
  *inptrp = inptr;
  *outptrp = outptr;
#ifdef UPDATE_PARAMS
  UPDATE_PARAMS;
#endif

  return result;
}


/* We remove the macro definitions so that we can include this file again
   for the definition of another function.  */
#undef MIN_NEEDED_INPUT
#undef MAX_NEEDED_INPUT
#undef MIN_NEEDED_OUTPUT
#undef MAX_NEEDED_OUTPUT
#undef LOOPFCT
#undef COUNT_CONVERTED
#undef BODY
#undef LOOPFCT
#undef EXTRA_LOOP_DECLS
#undef INIT_PARAMS
#undef UPDATE_PARAMS
