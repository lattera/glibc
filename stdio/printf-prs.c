/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <printf.h>
#include <stdlib.h>
#include <string.h>

#include "printf-parse.h"


size_t
parse_printf_format (fmt, n, argtypes)
      const char *fmt;
      size_t n;
      int *argtypes;
{
  size_t nargs;			/* Number of arguments.  */
  size_t max_ref_arg;		/* Highest index used in a positional arg.  */
  struct printf_spec spec;

  nargs = 0;
  max_ref_arg = 0;

  /* Search for format specifications.  */
  for (fmt = find_spec (fmt); *fmt != '\0'; fmt = spec.next_fmt)
    {
      /* Parse this spec.  */
      nargs += parse_one_spec (fmt, nargs, &spec, &max_ref_arg);

      /* If the width is determined by an argument this is an int.  */
      if (spec.width_arg != -1 && spec.width_arg < n)
	argtypes[spec.width_arg] = PA_INT;

      /* If the precision is determined by an argument this is an int.  */
      if (spec.prec_arg != -1 && spec.prec_arg < n)
	argtypes[spec.prec_arg] = PA_INT;

      if (spec.data_arg < n)
	switch (spec.ndata_args)
	  {
	  case 0:		/* No arguments.  */
	    break;
	  case 1:		/* One argument; we already have the type.  */
	    argtypes[spec.data_arg] = spec.data_arg_type;
	    break;
	  default:
	    /* We have more than one argument for this format spec.  We must
               call the arginfo function again to determine all the types.  */
	    (void) (*__printf_arginfo_table[spec.info.spec])
	      (&spec.info, n - spec.data_arg, &argtypes[spec.data_arg]);
	    break;
	  }
    }

  return MAX (nargs, max_ref_arg);
}
