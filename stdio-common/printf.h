/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_PRINTF_H

#define	_PRINTF_H	1
#include <features.h>

__BEGIN_DECLS

#define	__need_FILE
#include <stdio.h>
#define	__need_size_t
#include <stddef.h>


struct printf_info
{
  int prec;			/* Precision.  */
  int width;			/* Width.  */
  unsigned char spec;		/* Format letter.  */
  unsigned int is_long_double:1;/* L flag.  */
  unsigned int is_short:1;	/* h flag.  */
  unsigned int is_long:1;	/* l flag.  */
  unsigned int alt:1;		/* # flag.  */
  unsigned int space:1;		/* Space flag.  */
  unsigned int left:1;		/* - flag.  */
  unsigned int showsign:1;	/* + flag.  */
  unsigned int group:1;		/* ' flag.  */
  char pad;			/* Padding character.  */
};


/* Type of a printf specifier-handler function.
   STREAM is the FILE on which to write output.
   INFO gives information about the format specification.
   ARGS is a vector of pointers to the argument data;
   the number of pointers will be the number returned
   by the associated arginfo function for the same INFO.

   The function should return the number of characters written,
   or -1 for errors.  */

typedef int printf_function __P ((FILE *__stream,
				  __const struct printf_info *__info,
				  __const void **__const __args));

/* Type of a printf specifier-arginfo function.
   INFO gives information about the format specification.
   N, ARGTYPES, and return value are as for printf_parse_format.  */

typedef int printf_arginfo_function __P ((__const struct printf_info * __info,
					  size_t __n,
					  int *__argtypes));


/* Register FUNC to be called to format SPEC specifiers; ARGINFO must be
   specified to determine how many arguments a SPEC conversion requires and
   what their types are, even if your program never calls
   `parse_printf_format'.  */

extern int register_printf_function __P ((int __spec, printf_function __func,
					  printf_arginfo_function __arginfo));


/* Parse FMT, and fill in N elements of ARGTYPES with the
   types needed for the conversions FMT specifies.  Returns
   the number of arguments required by FMT.

   The ARGINFO function registered with a user-defined format is passed a
   `struct printf_info' describing the format spec being parsed.  A width
   or precision of INT_MIN means a `*' was used to indicate that the
   width/precision will come from an arg.  The function should fill in the
   array it is passed with the types of the arguments it wants, and return
   the number of arguments it wants.  */

extern size_t parse_printf_format __P ((__const char *__fmt,
					size_t __n,
					int *__argtypes));


/* Codes returned by `parse_printf_format' for basic types.

   These values cover all the standard format specifications.
   Users can add new values after PA_LAST for their own types.  */

enum
{				/* C type: */
  PA_INT,			/* int */
  PA_CHAR,			/* int, cast to char */
  PA_STRING,			/* const char *, a '\0'-terminated string */
  PA_POINTER,			/* void * */
  PA_FLOAT,			/* float */
  PA_DOUBLE,			/* double */
  PA_LAST
};

/* Flag bits that can be set in a type returned by `parse_printf_format'.  */
#define	PA_FLAG_MASK		0xff00
#define	PA_FLAG_LONG_LONG	(1 << 8)
#define	PA_FLAG_LONG_DOUBLE	PA_FLAG_LONG_LONG
#define	PA_FLAG_LONG		(1 << 9)
#define	PA_FLAG_SHORT		(1 << 10)
#define	PA_FLAG_PTR		(1 << 11)


__END_DECLS

#endif /* printf.h  */
