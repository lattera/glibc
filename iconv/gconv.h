/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* This header provides no interface for a user to the internals of
   the gconv implementation in the libc.  Therefore there is no use
   for these definitions beside for writing additional gconv modules.  */

#ifndef _GCONV_H
#define _GCONV_H	1

#include <features.h>
#include <wchar.h>
#define __need_size_t
#include <stddef.h>

/* ISO 10646 value used to signal invalid value.  */
#define UNKNOWN_10646_CHAR	((wchar_t) 0xfffd)

/* Error codes for gconv functions.  */
enum
{
  GCONV_OK = 0,
  GCONV_NOCONV,
  GCONV_NODB,
  GCONV_NOMEM,

  GCONV_EMPTY_INPUT,
  GCONV_FULL_OUTPUT,
  GCONV_ILLEGAL_INPUT,
  GCONV_INCOMPLETE_INPUT,

  GCONV_ILLEGAL_DESCRIPTOR,
  GCONV_INTERNAL_ERROR
};


/* Forward declarations.  */
struct gconv_step;
struct gconv_step_data;
struct gconv_loaded_object;


/* Type of a conversion function.  */
typedef int (*gconv_fct) __PMT ((struct gconv_step *,
				 struct gconv_step_data *, __const char **,
				 __const char *, size_t *, int));

/* Constructor and destructor for local data for conversion step.  */
typedef int (*gconv_init_fct) __PMT ((struct gconv_step *));
typedef void (*gconv_end_fct) __PMT ((struct gconv_step *));


/* Description of a conversion step.  */
struct gconv_step
{
  struct gconv_loaded_object *shlib_handle;
  __const char *modname;

  int counter;

  __const char *from_name;
  __const char *to_name;

  gconv_fct fct;
  gconv_init_fct init_fct;
  gconv_end_fct end_fct;

  /* Information about the number of bytes needed or produced in this
     step.  This helps optimizing the buffer sizes.  */
  int min_needed_from;
  int max_needed_from;
  int min_needed_to;
  int max_needed_to;

  /* Flag whether this is a stateful encoding or not.  */
  int stateful;

  void *data;		/* Pointer to step-local data.  */
};

/* Additional data for steps in use of conversion descriptor.  This is
   allocated by the `init' function.  */
struct gconv_step_data
{
  char *outbuf;		/* Output buffer for this step.  */
  char *outbufend;	/* Address of first byte after the output buffer.  */

  /* Is this the last module in the chain.  */
  int is_last;

  /* Counter for number of invocations of the module function for this
     descriptor.  */
  int invocation_counter;

  /* Flag whether this is an internal use of the module (in the mb*towc*
     and wc*tomb* functions) or regular with iconv(3).  */
  int internal_use;

  mbstate_t *statep;
  mbstate_t __state;	/* This element should not be used directly by
			   any module; always use STATEP!  */
};


/* Combine conversion step description with data.  */
typedef struct gconv_info
{
  size_t nsteps;
  struct gconv_step *steps;
  struct gconv_step_data *data;
} *gconv_t;

#endif /* gconv.h */
