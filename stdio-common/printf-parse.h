/* Internal header for parsing printf format strings.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of th GNU C Library.

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

#include <ctype.h>
#include <printf.h>
#include <string.h>
#include <stddef.h>

#define NDEBUG 1
#include <assert.h>

#define MAX(a,b)	({typeof(a) _a = (a); typeof(b) _b = (b);	      \
			  _a > _b ? _a : _b; })
#define MIN(a,b)	({typeof(a) _a = (a); typeof(b) _b = (b);	      \
			  _a < _b ? _a : _b; })

struct printf_spec
  {
    /* Information parsed from the format spec.  */ 
    struct printf_info info;

    /* Pointers into the format string for the end of this format
       spec and the next (or to the end of the string if no more).  */
    const char *end_of_fmt, *next_fmt;

    /* Position of arguments for precision and width, or -1 if `info' has
       the constant value.  */
    int prec_arg, width_arg;

    int data_arg;		/* Position of data argument.  */
    int data_arg_type;		/* Type of first argument.  */
    /* Number of arguments consumed by this format specifier.  */
    size_t ndata_args;
  };


/* The various kinds off arguments that can be passed to printf.  */
union printf_arg
  {
    unsigned char pa_char;
    short int pa_short_int;
    int pa_int;
    long int pa_long_int;
    long long int pa_long_long_int;
    unsigned short int pa_u_short_int;
    unsigned int pa_u_int;
    unsigned long int pa_u_long_int;
    unsigned long long int pa_u_long_long_int;
    float pa_float;
    double pa_double;
    long double pa_long_double;
    const char *pa_string;
    void *pa_pointer;
  };


/* Read a simple integer from a string and update the string pointer.
   It is assumed that the first character is a digit.  */
static inline unsigned int
read_int (const char * *pstr)
{
  unsigned int retval = **pstr - '0';

  while (isdigit (*++(*pstr)))
    {
      retval *= 10;
      retval += **pstr - '0';
    }

  return retval;
}



/* Find the next spec in FORMAT, or the end of the string.  Returns
   a pointer into FORMAT, to a '%' or a '\0'.  */
static inline const char *
find_spec (const char *format)
{
  while (*format != '\0' && *format != '%')
    {
      int len;

      if (isascii (*format) || (len = mblen (format, MB_CUR_MAX)) <= 0)
	++format;
      else
	format += len;
    }
  return format;
}


/* This is defined in reg-printf.c.  */
extern printf_arginfo_function **__printf_arginfo_table;


/* FORMAT must point to a '%' at the beginning of a spec.  Fills in *SPEC
   with the parsed details.  POSN is the number of arguments already
   consumed.  At most MAXTYPES - POSN types are filled in TYPES.  Return
   the number of args consumed by this spec; *MAX_REF_ARG is updated so it
   remains the highest argument index used.  */
static inline size_t
parse_one_spec (const char *format, size_t posn, struct printf_spec *spec,
		size_t *max_ref_arg)
{
  unsigned int n;
  size_t nargs = 0;

  /* Skip the '%'.  */
  ++format;

  /* Clear information structure.  */
  spec->data_arg = -1;
  spec->info.alt = 0;
  spec->info.space = 0;
  spec->info.left = 0;
  spec->info.showsign = 0;
  spec->info.group = 0;
  spec->info.pad = ' ';

  /* Test for positional argument.  */
  if (isdigit (*format))
    {
      const char *begin = format;

      n = read_int (&format);

      if (n > 0 && *format == '$')
	/* Is positional parameter.  */
	{
	  ++format;		/* Skip the '$'.  */
	  spec->data_arg = n - 1;
	  *max_ref_arg = MAX (*max_ref_arg, n);
	}
      else
	/* Oops; that was actually the width and/or 0 padding flag.
	   Step back and read it again.  */
	format = begin;
    }

  /* Check for spec modifiers.  */
  while (*format == ' ' || *format == '+' || *format == '-' ||
	 *format == '#' || *format == '0' || *format == '\'')
    switch (*format++)
      {
      case ' ':
	/* Output a space in place of a sign, when there is no sign.  */
	spec->info.space = 1;
	break;
      case '+':
	/* Always output + or - for numbers.  */
	spec->info.showsign = 1;
	break;
      case '-':
	/* Left-justify things.  */
	spec->info.left = 1;
	break;
      case '#':
	/* Use the "alternate form":
	   Hex has 0x or 0X, FP always has a decimal point.  */
	spec->info.alt = 1;
	break;
      case '0':
	/* Pad with 0s.  */
	spec->info.pad = '0';
	break;
      case '\'':
	/* Show grouping in numbers if the locale information
	   indicates any.  */
	spec->info.group = 1;
	break;
      }
  if (spec->info.left)
    spec->info.pad = ' ';

  /* Get the field width.  */
  spec->width_arg = -1;
  spec->info.width = 0;
  if (*format == '*')
    {
      /* The field width is given in an argument.
	 A negative field width indicates left justification.  */
      const char *begin = ++format;

      if (isdigit (*format))
	{
	  /* The width argument might be found in a positional parameter.  */
	  n = read_int (&format);

	  if (n > 0 && *format == '$')
	    {
	      spec->width_arg = n - 1;
	      *max_ref_arg = MAX (*max_ref_arg, n);
	      ++format;		/* Skip '$'.  */
	    }
	}

      if (spec->width_arg < 0)
	{
	  /* Not in a positional parameter.  Consume one argument.  */
	  spec->width_arg = posn++;
	  ++nargs;
	  format = begin;	/* Step back and reread.  */
	}
    }
  else if (isdigit (*format))
    /* Constant width specification.  */
    spec->info.width = read_int (&format);

  /* Get the precision.  */
  spec->prec_arg = -1;
  /* -1 means none given; 0 means explicit 0.  */
  spec->info.prec = -1;
  if (*format == '.')
    {
      ++format;
      if (*format == '*')
	{
	  /* The precision is given in an argument.  */
	  const char *begin = ++format;

	  if (isdigit (*format))
	    {
	      n = read_int (&format);

	      if (n > 0 && *format == '$')
		{
		  spec->prec_arg = n - 1;
		  *max_ref_arg = MAX (*max_ref_arg, n);
		  ++format;
		}
	    }

	  if (spec->prec_arg < 0)
	    {
	      /* Not in a positional parameter.  */
	      spec->prec_arg = posn++;
	      ++nargs;
	      format = begin;
	    }
	}
      else if (isdigit (*format))
	spec->info.prec = read_int (&format);
      else
	/* "%.?" is treated like "%.0?".  */
	spec->info.prec = 0;

      /* If there was a precision specified, ignore the 0 flag and always
	 pad with spaces.  */
      spec->info.pad = ' ';
    }

  /* Check for type modifiers.  */
#define is_longlong is_long_double
  spec->info.is_long_double = 0;
  spec->info.is_short = 0;
  spec->info.is_long = 0;

  while (*format == 'h' || *format == 'l' || *format == 'L' ||
	 *format == 'Z' || *format == 'q')
    switch (*format++)
      {
      case 'h':
	/* int's are short int's.  */
	spec->info.is_short = 1;
	break;
      case 'l':
	if (spec->info.is_long)
	  /* A double `l' is equivalent to an `L'.  */
	  spec->info.is_longlong = 1;
	else
	  /* int's are long int's.  */
	  spec->info.is_long = 1;
	break;
      case 'L':
	/* double's are long double's, and int's are long long int's.  */
	spec->info.is_long_double = 1;
	break;
      case 'Z':
	/* int's are size_t's.  */
	assert (sizeof(size_t) <= sizeof(unsigned long long int));
	spec->info.is_longlong = sizeof(size_t) > sizeof(unsigned long int);
	spec->info.is_long = sizeof(size_t) > sizeof(unsigned int);
	break;
      case 'q':
	/* 4.4 uses this for long long.  */
	spec->info.is_longlong = 1;
	break;
      }

  /* Get the format specification.  */
  spec->info.spec = *format++;
  if (__printf_arginfo_table != NULL &&
      __printf_arginfo_table[spec->info.spec] != NULL)
    /* We don't try to get the types for all arguments if the format
       uses more than one.  The normal case is covered though.  */
    spec->ndata_args = (*__printf_arginfo_table[spec->info.spec])
      (&spec->info, 1, &spec->data_arg_type);
  else
    {
      /* Find the data argument types of a built-in spec.  */
      spec->ndata_args = 1;

      switch (spec->info.spec)
	{
	case 'i':
	case 'd':
	case 'u':
	case 'o':
	case 'X':
	case 'x':
	  if (spec->info.is_longlong)
	    spec->data_arg_type = PA_INT|PA_FLAG_LONG_LONG;
	  else if (spec->info.is_long)
	    spec->data_arg_type = PA_INT|PA_FLAG_LONG;
	  else if (spec->info.is_short)
	    spec->data_arg_type = PA_INT|PA_FLAG_SHORT;
	  else
	    spec->data_arg_type = PA_INT;
	  break;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	  if (spec->info.is_long_double)
	    spec->data_arg_type = PA_DOUBLE|PA_FLAG_LONG_DOUBLE;
	  else
	    spec->data_arg_type = PA_DOUBLE;
	  break;
	case 'c':
	  spec->data_arg_type = PA_CHAR;
	  break;
	case 's':
	  spec->data_arg_type = PA_STRING;
	  break;
	case 'p':
	  spec->data_arg_type = PA_POINTER;
	  break;
	case 'n':
	  spec->data_arg_type = PA_INT|PA_FLAG_PTR;
	  break;

	case 'm':
	default:
	  /* An unknown spec will consume no args.  */
	  spec->ndata_args = 0;
	  break;
	}

      if (spec->data_arg == -1 && spec->ndata_args > 0) 
	{
	  /* There are args consumed, but no positional spec.
	     Use the next sequential arg position.  */
	  spec->data_arg = posn;
	  posn += spec->ndata_args;
	  nargs += spec->ndata_args;
	}
    }

  if (spec->info.spec == '\0')
    /* Format ended before this spec was complete.  */
    spec->end_of_fmt = spec->next_fmt = format - 1;
  else
    {
      /* Find the next format spec.  */
      spec->end_of_fmt = format;
      spec->next_fmt = find_spec (format);
    }

  return nargs;
}
