/* Conversion to and from the various ISO 646 CCS.
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

/* The implementation of the conversion which can be performed by this
   module are not very sophisticated and not tuned at all.  There are
   zillions of ISO 646 derivates and supporting them all in a separate
   module is overkill since these coded character sets are hardly ever
   used anymore (except ANSI_X3.4-1968 == ASCII, which is compatible
   with ISO 8859-1).  The European variants are superceded by the
   various ISO 8859-? standards and the Asian variants are embedded in
   larger character sets.  Therefore this implementation is simply
   here to make it possible to do the conversion if it is necessary.
   The cost in the gconv-modules file is set to `2' and therefore
   allows one to easily provide a tuned implementation in case this
   proofs to be necessary.  */

#include <gconv.h>
#include <stdint.h>
#include <string.h>

/* Definitions used in the body of the `gconv' function.  */
#define FROM_LOOP		from_ascii
#define TO_LOOP			to_ascii
#define DEFINE_INIT		0
#define DEFINE_FINI		0
#define MIN_NEEDED_FROM		1
#define MIN_NEEDED_TO		4
#define FROM_DIRECTION		dir == from_iso646
#define PREPARE_LOOP \
  enum direction dir = ((struct iso646_data *) step->data)->dir;	      \
  enum variant var = ((struct iso646_data *) step->data)->var;
#define EXTRA_LOOP_ARGS		, var


/* Direction of the transformation.  */
enum direction
{
  illegal_dir,
  to_iso646,
  from_iso646
};

enum variant
{
  illegal_var,
  US,		/* ANSI_X3.4-1968 */
  GB,		/* BS_4730 */
};

struct iso646_data
{
  enum direction dir;
  enum variant var;
};


int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  struct iso646_data *new_data;
  enum direction dir;
  enum variant var;
  int result;

  if (__strcasestr (step->from_name, "ANSI_X3.4-1968") != NULL)
    {
      dir = from_iso646;
      var = US;
    }
  else if (__strcasestr (step->from_name, "BS_4730") != NULL)
    {
      dir = from_iso646;
      var = GB;
    }
  else if (__strcasestr (step->to_name, "ANSI_X3.4-1968") != NULL)
    {
      dir = to_iso646;
      var = US;
    }
  else if (__strcasestr (step->to_name, "BS_4730") != NULL)
    {
      dir = to_iso646;
      var = GB;
    }
  else
    {
      dir = illegal_dir;
      var = illegal_var;
    }

  result = GCONV_NOCONV;
  if (dir != illegal_dir
      && ((new_data
	   = (struct iso646_data *) malloc (sizeof (struct iso646_data)))
	  != NULL))
    {
      new_data->dir = dir;
      new_data->var = var;
      step->data = new_data;
      result = GCONV_OK;
    }

  step->min_needed_from = MIN_NEEDED_FROM;
  step->max_needed_from = MIN_NEEDED_FROM;
  step->min_needed_to = MIN_NEEDED_TO;
  step->max_needed_to = MIN_NEEDED_TO;

  step->stateful = 0;

  return result;
}


void
gconv_end (struct gconv_step *data)
{
  free (data->data);
}


/* First define the conversion function from ASCII to UCS4.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_FROM
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_TO
#define LOOPFCT			FROM_LOOP
#define BODY \
  {									      \
    uint32_t ch;							      \
    int failure = GCONV_OK;						      \
									      \
    switch (*inptr)							      \
      {									      \
      case '\x23':							      \
	if (var == GB)							      \
	  ch = 0xa3;							      \
	else								      \
	  ch = 0x23;							      \
	break;								      \
      case '\x7e':							      \
	if (var == GB)							      \
	  ch = 0x203e;							      \
	else								      \
	  ch = 0x7e;							      \
	break;								      \
      default:								      \
	ch = *inptr;							      \
	break;								      \
      case '\x80' ... '\xff':						      \
	/* Illegal character.  */					      \
	failure = GCONV_ILLEGAL_INPUT;					      \
	ch = '\0';	/* OK, gcc, here I initialize the variable.  */	      \
	break;								      \
      }									      \
									      \
    /* Hopefully gcc can recognize that the following `if' is only true	      \
       when we reach the default case in the `switch' statement.  */	      \
    if (failure == GCONV_ILLEGAL_INPUT)					      \
      {									      \
	/* Exit the loop with an error.  */				      \
	result = failure;						      \
	break;								      \
      }									      \
    *((uint32_t *) outptr)++ = ch;					      \
    ++inptr;								      \
  }
#define EXTRA_LOOP_DECLS	, enum variant var
#include <iconv/loop.c>


/* Next, define the other direction.  */
#define MIN_NEEDED_INPUT	MIN_NEEDED_TO
#define MIN_NEEDED_OUTPUT	MIN_NEEDED_FROM
#define LOOPFCT			TO_LOOP
#define BODY \
  {									      \
    unsigned char ch;							      \
    int failure = GCONV_OK;						      \
									      \
    do									      \
      {									      \
	switch (*((uint32_t *) inptr))					      \
	  {								      \
	  case 0x23:							      \
	    if (var == GB)						      \
	      break;							      \
	    ch = 0x23;							      \
	    continue;							      \
	  case 0x7e:							      \
	    if (var == GB)						      \
	      break;							      \
	    ch = 0x7e;							      \
	    continue;							      \
	  case 0xa3:							      \
	    if (var != GB)						      \
	      break;							      \
	    ch = 0x23;							      \
	    continue;							      \
	  case 0x203e:							      \
	    if (var != GB)						      \
	      break;							      \
	    ch = 0x7e;							      \
	    continue;							      \
	  default:							      \
	    if (*((uint32_t *) inptr) > 0x7f)				      \
	      break;							      \
	    ch = (unsigned char) *((uint32_t *) inptr);			      \
	    continue;							      \
	  }								      \
	/* When we come to this place we saw an illegal character.  */	      \
	failure = GCONV_ILLEGAL_INPUT;					      \
	ch = '\0';	/* OK, gcc, here I initialize the variable.  */	      \
      }									      \
    while (0);								      \
									      \
    /* Hopefully gcc can recognize that the following `if' is only true	      \
       when we fall through the `switch' statement.  */			      \
    if (failure == GCONV_ILLEGAL_INPUT)					      \
      {									      \
	/* Exit the loop with an error.  */				      \
	result = failure;						      \
	break;								      \
      }									      \
    *outptr++ = ch;							      \
    inptr += 4;								      \
  }
#define EXTRA_LOOP_DECLS	, enum variant var
#include <iconv/loop.c>


/* Now define the toplevel functions.  */
#include <iconv/skeleton.c>
