/* Skeleton for a converison module.
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

/* This file can be included to provide definitions of several things
   many modules have in common.  It can be customized using the following
   macros:

     DEFINE_INIT	define the default initializer.  This requires the
			following symbol to be defined.

     CHARSET_NAME	string with official name of the coded character
			set (in all-caps)

     DEFINE_FINI	define the default destructor function.

     MIN_NEEDED_FROM	minimal number of bytes needed for the from-charset.
     MIN_NEEDED_TO	likewise for the to-charset.

     MAX_NEEDED_FROM	maximal number of bytes needed for the from-charset.
			This macro is optional, it defaults to MIN_NEEDED_FROM.
     MAX_NEEDED_TO	likewise for the to-charset.

     DEFINE_DIRECTION_OBJECTS
			two objects will be defined to be used when the
			`gconv' function must only distinguish two
			directions.  This is implied by DEFINE_INIT.
			If this macro is not defined the following
			macro must be available.

     FROM_DIRECTION	this macro is supposed to return a value != 0
			if we convert from the current character set,
			otherwise it return 0.

     EMIT_SHIFT_TO_INIT	this symbol is optional.  If it is defined it
			defines some code which writes out a sequence
			of characters which bring the current state into
			the initial state.

     FROM_LOOP		name of the function implementing the conversion
			from the current characters.
     TO_LOOP		likewise for the other direction

     RESET_STATE	in case of an error we must reset the state for
			the rerun so this macro must be defined for
			stateful encodings.  It takes an argument which
			is nonzero when saving.

     RESET_INPUT_BUFFER	If the input character sets allow this the macro
			can be defined to reset the input buffer pointers
			to cover only those characters up to the error.

     FUNCTION_NAME	if not set the conversion function is named `gconv'.

     PREPARE_LOOP	optional code preparing the conversion loop.  Can
			contain variable definitions.
     END_LOOP		also optional, may be used to store information

     EXTRA_LOOP_ARGS	optional macro specifying extra arguments passed
			to loop function.
 */

#include <assert.h>
#include <gconv.h>
#include <string.h>
#define __need_size_t
#define __need_NULL
#include <stddef.h>


/* The direction objects.  */
#if DEFINE_DIRECTION_OBJECTS || DEFINE_INIT
static int from_object;
static int to_object;

# ifndef FROM_DIRECTION
#  define FROM_DIRECTION step->data == &from_object
# endif
#else
# ifndef FROM_DIRECTION
#  error "FROM_DIRECTION must be provided if direction objects are not used"
# endif
#endif


/* How many bytes are needed at most for the from-charset.  */
#ifndef MAX_NEEDED_FROM
# define MAX_NEEDED_FROM	MIN_NEEDED_FROM
#endif

/* Same for the to-charset.  */
#ifndef MAX_NEEDED_TO
# define MAX_NEEDED_TO		MIN_NEEDED_TO
#endif


/* For conversions from a fixed width character sets to another fixed width
   character set we we can define RESET_INPUT_BUFFER is necessary.  */
#if !defined RESET_INPUT_BUFFER && !defined SAVE_RESET_STATE
# if MIN_NEEDED_FROM == MAX_NEEDED_FROM && MIN_NEEDED_TO == MAX_NEEDED_TO
/* We have to used these `if's here since the compiler cannot know that
   (outbuf - outerr) is always divisible by MIN_NEEDED_TO.  */
#  define RESET_INPUT_BUFFER \
  if (MIN_NEEDED_FROM % MIN_NEEDED_TO == 0)				      \
    *inbuf -= (outbuf - outerr) * (MIN_NEEDED_FROM / MIN_NEEDED_TO);	      \
  else if (MIN_NEEDED_TO % MIN_NEEDED_FROM == 0)			      \
    *inbuf -= (outbuf - outerr) / (MIN_NEEDED_TO / MIN_NEEDED_FROM);	      \
  else									      \
    *inbuf -= ((outbuf - outerr) / MIN_NEEDED_TO) * MIN_NEEDED_FROM
# endif
#endif


/* The default init function.  It simply matches the name and initializes
   the step data to point to one of the objects above.  */
#if DEFINE_INIT
# ifndef CHARSET_NAME
#  error "CHARSET_NAME not defined"
# endif

int
gconv_init (struct gconv_step *step)
{
  /* Determine which direction.  */
  if (__strcasecmp (step->from_name, CHARSET_NAME) == 0)
    step->data = &from_object;
  else if (__strcasecmp (step->to_name, CHARSET_NAME) == 0)
    step->data = &to_object;
  else
    return GCONV_NOCONV;

  if (step->data == &from_object)
    {
      step->min_needed_from = MIN_NEEDED_FROM;
      step->max_needed_from = MAX_NEEDED_FROM;
      step->min_needed_to = MIN_NEEDED_TO;
      step->max_needed_to = MAX_NEEDED_TO;
    }
  else
    {
      step->min_needed_from = MIN_NEEDED_TO;
      step->max_needed_from = MAX_NEEDED_TO;
      step->min_needed_to = MIN_NEEDED_FROM;
      step->max_needed_to = MAX_NEEDED_FROM;
    }

#ifdef RESET_STATE
  step->stateful = 1;
#else
  step->stateful = 0;
#endif

  return GCONV_OK;
}
#endif


/* The default destructor function does nothing in the moment and so
   be define it at all.  But we still provide the macro just in case
   we need it some day.  */
#if DEFINE_FINI
#endif


/* If no arguments have to passed to the loop function define the macro
   as empty.  */
#ifndef EXTRA_LOOP_ARGS
# define EXTRA_LOOP_ARGS
#endif


/* This is the actual conversion function.  */
#ifndef FUNCTION_NAME
# define FUNCTION_NAME	gconv
#endif

int
FUNCTION_NAME (struct gconv_step *step, struct gconv_step_data *data,
	       const char **inbuf, const char *inbufend, size_t *written,
	       int do_flush)
{
  struct gconv_step *next_step = step + 1;
  struct gconv_step_data *next_data = data + 1;
  gconv_fct fct = next_step->fct;
  int status;

  /* If the function is called with no input this means we have to reset
     to the initial state.  The possibly partly converted input is
     dropped.  */
  if (do_flush)
    {
      /* Call the steps down the chain if there are any.  */
      if (data->is_last)
	status = GCONV_OK;
      else
	{
#ifdef EMIT_SHIFT_TO_INIT
	  status = GCONV_OK;

	  EMIT_SHIFT_TO_INIT;

	  if (status == GCONV_OK)
#endif
	    /* Give the modules below the same chance.  */
	    status = (*fct) (next_step, next_data, NULL, NULL, written, 1);
	}
    }
  else
    {
      /* We preserve the initial values of the pointer variables.  */
      const char *inptr = *inbuf;
      char *outbuf = data->outbuf;
      char *outend = data->outbufend;
      char *outptr;

      /* This variable is used to count the number of characters we
	 actually converted.  */
      size_t converted = 0;

#ifdef PREPARE_LOOP
      PREPARE_LOOP
#endif

      do
	{
	  /* Remember the start value for this round.  */
	  inptr = *inbuf;
	  /* The outbuf buffer is empty.  */
	  outptr = outbuf;

#ifdef SAVE_RESET_STATE
	  SAVE_RESET_STATE (1);
#endif

	  if (FROM_DIRECTION)
	    /* Run the conversion loop.  */
	    status = FROM_LOOP ((const unsigned char **) inbuf,
				(const unsigned char *) inbufend,
				(unsigned char **) &outbuf,
				(unsigned char *) outend,
				data->statep, step->data, &converted
				EXTRA_LOOP_ARGS);
	  else
	    /* Run the conversion loop.  */
	    status = TO_LOOP ((const unsigned char **) inbuf,
			      (const unsigned char *) inbufend,
			      (unsigned char **) &outbuf,
			      (unsigned char *) outend,
			      data->statep, step->data, &converted
			      EXTRA_LOOP_ARGS);

	  /* If this is the last step leave the loop, there is nothgin
             we can do.  */
	  if (data->is_last)
	    {
	      /* Store information about how many bytes are available.  */
	      data->outbuf = outbuf;

	      /* Remember how many characters we converted.  */
	      *written += converted;

	      break;
	    }

	  /* Write out all output which was produced.  */
	  if (outbuf > outptr)
	    {
	      const char *outerr = data->outbuf;
	      int result;

	      result = (*fct) (next_step, next_data, &outerr, outbuf,
			       written, 0);

	      if (result != GCONV_EMPTY_INPUT)
		{
		  if (outerr != outbuf)
		    {
#ifdef RESET_INPUT_BUFFER
		      RESET_INPUT_BUFFER;
#else
		      /* We have a problem with the in on of the functions
			 below.  Undo the conversion upto the error point.  */
		      size_t nstatus;

		      /* Reload the pointers.  */
		      *inbuf = inptr;
		      outbuf = outptr;

		      /* Reset the state.  */
# ifdef SAVE_RESET_STATE
		      SAVE_RESET_STATE (0);
# endif

		      if (FROM_DIRECTION)
			/* Run the conversion loop.  */
			nstatus = FROM_LOOP ((const unsigned char **) inbuf,
					     (const unsigned char *) inbufend,
					     (unsigned char **) &outbuf,
					     (unsigned char *) outerr,
					     data->statep, step->data,
					     &converted EXTRA_LOOP_ARGS);
		      else
			/* Run the conversion loop.  */
			nstatus = TO_LOOP ((const unsigned char **) inbuf,
					   (const unsigned char *) inbufend,
					   (unsigned char **) &outbuf,
					   (unsigned char *) outerr,
					   data->statep, step->data,
					   &converted EXTRA_LOOP_ARGS);

		      /* We must run out of output buffer space in this
			 rerun.  */
		      assert (outbuf == outerr);
		      assert (nstatus == GCONV_FULL_OUTPUT);
#endif	/* reset input buffer */
		    }

		  /* Change the status.  */
		  status = result;
		}
	      else
		/* All the output is consumed, we can make another run
		   if everything was ok.  */
		if (status == GCONV_FULL_OUTPUT)
		  status = GCONV_OK;
	    }
	}
      while (status == GCONV_OK);

#ifdef END_LOOP
      END_LOOP
#endif
    }

  return status;
}

#undef DEFINE_INIT
#undef CHARSET_NAME
#undef DEFINE_FINI
#undef MIN_NEEDED_FROM
#undef MIN_NEEDED_TO
#undef MAX_NEEDED_FROM
#undef MAX_NEEDED_TO
#undef DEFINE_DIRECTION_OBJECTS
#undef FROM_DIRECTION
#undef EMIT_SHIFT_TO_INIT
#undef FROM_LOOP
#undef TO_LOOP
#undef RESET_STATE
#undef RESET_INPUT_BUFFER
#undef FUNCTION_NAME
#undef PREPARE_LOOP
#undef END_LOOP
