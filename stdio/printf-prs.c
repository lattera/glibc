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

#include <ansidecl.h>
#include <stdio.h>
#include <printf.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#ifdef	__GNUC__
#define	HAVE_LONGLONG
#endif

extern printf_arginfo_function *__printf_arginfo_table[];

size_t
DEFUN(parse_printf_format, (fmt, n, argtypes),
      CONST char *fmt AND size_t n AND int *argtypes)
{
  register CONST char *f;
  size_t need = 0;

  for (f = strchr (fmt, '%'); f != NULL; f = strchr (f, '%'))
    {
      struct printf_info info;
      printf_arginfo_function *arginfo;

      ++f;

      info.space = info.showsign = info.left = info.alt = info.group = 0;
      info.pad = ' ';
      while (*f == ' ' || *f == '+' || *f == '-' || *f == '#' || *f == '0' ||
	     *f == '\'')
	switch (*f++)
	  {
	  case ' ':
	    info.space = 1;
	    break;
	  case '+':
	    info.showsign = 1;
	    break;
	  case '-':
	    info.left = 1;
	    break;
	  case '#':
	    info.alt = 1;
	    break;
	  case '\'':
	    info.group = 1;
	    break;
	  case '0':
	    info.pad = '0';
	    break;
	  }
      if (info.left)
	info.pad = ' ';

      /* Get the field width.  */
      if (*f == '*')
	{
	  if (++need < n)
	    *argtypes++ = PA_INT;
	  info.width = INT_MIN;
	  ++f;
	}
      else
	{
	  info.width = 0;
	  while (isdigit(*f))
	    {
	      info.width *= 10;
	      info.width += *f++ - '0';
	    }
	}

      /* Get the precision.  */
      /* -1 means none given; 0 means explicit 0.  */
      info.prec = -1;
      if (*f == '.')
	{
	  ++f;
	  if (*f == '*')
	    {
	      /* The precision is given in an argument.  */
	      if (++need < n)
		*argtypes++ = PA_INT;
	      info.prec = INT_MIN;
	      ++f;
	    }
	  else if (isdigit(*f))
	    {
	      info.prec = 0;
	      while (*f != '\0' && isdigit(*f))
		{
		  info.prec *= 10;
		  info.prec += *f++ - '0';
		}
	    }
	}

      /* Check for type modifiers.  */
      info.is_short = info.is_long = info.is_long_double = 0;
      while (*f == 'h' || *f == 'l' || *f == 'L')
	switch (*f++)
	  {
	  case 'h':
	    /* int's are short int's.  */
	    info.is_short = 1;
	    break;
	  case 'l':
#ifdef	HAVE_LONGLONG
	    if (info.is_long)
	      /* A double `l' is equivalent to an `L'.  */
	      info.is_long_double = 1;
	    else
#endif
	      /* int's are long int's.  */
	      info.is_long = 1;
	    break;
	  case 'L':
	    /* double's are long double's, and int's are long long int's.  */
	    info.is_long_double = 1;
	    break;
	  }

      if (*f == '\0')
	return need;

      info.spec = *f++;

      arginfo = __printf_arginfo_table[info.spec];
      if (arginfo != NULL)
	{
	  size_t nargs
	    = (*arginfo) (&info, need > n ? 0 : n - need, argtypes);
	  need += nargs;
	  argtypes += nargs;
	}
      else
	{
	  int type;
	  switch (info.spec)
	    {
	    case 'i':
	    case 'd':
	    case 'u':
	    case 'o':
	    case 'X':
	    case 'x':
	      type = PA_INT;
	      break;

	    case 'e':
	    case 'E':
	    case 'f':
	    case 'g':
	    case 'G':
	      type = PA_DOUBLE;
	      break;

	    case 'c':
	      type = PA_CHAR;
	      break;

	    case 's':
	      type = PA_STRING;
	      break;

	    case 'p':
	      type = PA_POINTER;
	      break;

	    case 'n':
	      type = PA_INT | PA_FLAG_PTR;
	      break;

	    default:
	      /* No arg for an unknown spec.  */
	      continue;
	    }

	  if (info.is_long_double)
	    type |= PA_FLAG_LONG_DOUBLE;
	  if (info.is_long)
	    type |= PA_FLAG_LONG;
	  if (info.is_short)
	    type |= PA_FLAG_SHORT;

	  if (++need < n)
	    *argtypes++ = type;
	}
    }

  return need;
}
