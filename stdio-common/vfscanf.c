/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include "../locale/localeinfo.h"
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef	__GNUC__
#define	HAVE_LONGLONG
#define	LONGLONG	long long
#else
#define	LONGLONG	long
#endif

#ifdef USE_IN_LIBIO
# include <libioP.h>
# include <libio.h>

/* Those are flags in the conversion format. */
# define LONG		0x01	/* l: long or double */
# define LONGDBL	0x02	/* L: long long or long double */
# define SHORT		0x04	/* h: short */
# define SUPPRESS	0x08	/* suppress assignment */
# define POINTER	0x10	/* weird %p pointer (`fake hex') */
# define NOSKIP		0x20	/* do not skip blanks */
# define WIDTH		0x40	/* width */


# define va_list	_IO_va_list
# define ungetc(c, s)	_IO_ungetc (c, s)
# define inchar()	((c = _IO_getc(s)), ++read_in, c)
# define conv_error()	return ((errp != NULL && (*errp |= 2)), \
				(c == EOF || _IO_ungetc(c, s)), done)

# define input_error()	return ((errp != NULL && (*errp |= 1)), \
				done == 0 ? EOF : done)
# define memory_error()	return ((errno = ENOMEM), EOF)
# define ARGCHECK(s, format)						     \
  do									     \
    {									     \
      /* Check file argument for consistence.  */			     \
      CHECK_FILE (s, -1);						     \
      if (s->_flags & _IO_NO_READS || format == NULL)			     \
       {								     \
         MAYBE_SET_EINVAL;						     \
         return -1;							     \
       }								     \
    } while (0)
#else
# define inchar()	((c = getc(s)) == EOF ? EOF : (++read_in, c))
# define conv_error()	return (ungetc(c, s), done)
# define input_error()	return (done == 0 ? EOF : done)
# define memory_error()	return ((errno = ENOMEM), EOF)
# define ARGCHECK(s, format)						     \
  do									     \
    {									     \
      /* Check file argument for consistence.  */			     \
      if (!__validfp (s) || !s->__mode.__read || format == NULL)	     \
	{								     \
	  errno = EINVAL;						     \
	  return -1;							     \
	}								     \
    } while (0)
#endif


/* Read formatted input from S according to the format string
   FORMAT, using the argument list in ARG.
   Return the number of assignments made, or -1 for an input error.  */
#ifdef USE_IN_LIBIO
int
_IO_vfscanf (s, format, argptr, errp)
     _IO_FILE *s;
     const char *format;
     _IO_va_list argptr;
     int *errp;
#else
int
__vfscanf (FILE *s, const char *format, va_list argptr)
#endif
{
  va_list arg = (va_list) argptr;

  register const char *f = format;
  register char fc;		/* Current character of the format.  */
  register size_t done = 0;	/* Assignments done.  */
  register size_t read_in = 0;	/* Chars read in.  */
  register int c;		/* Last char read.  */
  register int do_assign;	/* Whether to do an assignment.  */
  register int width;		/* Maximum field width.  */
  int group_flag;		/* %' modifier flag.  */
#ifdef USE_IN_LIBIO
  int flags;			/* Trace flags for current format element.  */
#endif

  /* Type modifiers.  */
  int is_short, is_long, is_long_double;
#ifdef	HAVE_LONGLONG
  /* We use the `L' modifier for `long long int'.  */
# define is_longlong	is_long_double
#else
# define is_longlong	0
#endif
  int malloc_string;		/* Args are char ** to be filled in.  */
  /* Status for reading F-P nums.  */
  char got_dot, got_e;
  /* If a [...] is a [^...].  */
  char not_in;
  /* Base for integral numbers.  */
  int base;
  /* Signedness for integral numbers.  */
  int number_signed;
  /* Integral holding variables.  */
  union
    {
      long long int q;
      unsigned long long int uq;
      long int l;
      unsigned long int ul;
    } num;
  /* Character-buffer pointer.  */
  register char *str, **strptr;
  size_t strsize;
  /* Workspace.  */
  char work[200];
  char *w;			/* Pointer into WORK.  */
  wchar_t decimal;		/* Decimal point character.  */

  ARGCHECK (s, format);

  /* Figure out the decimal point character.  */
  if (mbtowc (&decimal, _NL_CURRENT (LC_NUMERIC, DECIMAL_POINT),
	      strlen (_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT))) <= 0)
    decimal = (wchar_t) *_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);

  c = inchar ();

  /* Run through the format string.  */
  while (*f != '\0')
    {
      unsigned int argpos;
      /* Extract the next argument, which is of type TYPE.
	 For a %N$... spec, this is the Nth argument from the beginning;
	 otherwise it is the next argument after the state now in ARG.  */
#if 0
      /* XXX Possible optimization.  */
# define ARG(type)	(argpos == 0 ? va_arg (arg, type) :		      \
			 ({ va_list arg = (va_list) argptr;		      \
			    arg = (va_list) ((char *) arg		      \
					     + (argpos - 1)		      \
					     * __va_rounded_size (void *));   \
			    va_arg (arg, type);				      \
			 }))
#else
# define ARG(type)	(argpos == 0 ? va_arg (arg, type) :		      \
			 ({ unsigned int pos = argpos;			      \
			    va_list arg = (va_list) argptr;		      \
			    while (--pos > 0)				      \
			      (void) va_arg (arg, void *);		      \
			    va_arg (arg, type);				      \
			  }))
#endif

      if (!isascii (*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen (f, strlen (f));
	  if (len > 0)
	    {
	      while (len-- > 0)
		if (c == EOF)
		  input_error ();
		else if (c == *f++)
		  (void) inchar ();
		else
		  conv_error ();
	      continue;
	    }
	}

      fc = *f++;
      if (fc != '%')
	{
	  /* Characters other than format specs must just match.  */
	  if (c == EOF)
	    input_error ();
	  if (isspace (fc))
	    {
	      /* Whitespace characters match any amount of whitespace.  */
	      while (isspace (c))
		inchar ();
	      continue;
	    }
	  else if (c == fc)
	    (void) inchar ();
	  else
	    conv_error ();
	  continue;
	}

#ifdef USE_IN_LIBIO
      /* That is the start of the coversion string. */
      flags = 0;
#endif

      /* Initialize state of modifiers.  */
      argpos = 0;
      do_assign = 1;
      group_flag = 0;
      is_short = is_long = is_long_double = malloc_string = 0;

      /* Check for a positional parameter specification.  */
      if (isdigit (*f))
	{
	  argpos = *f++ - '0';
	  while (isdigit (*f))
	    argpos = argpos * 10 + (*f++ - '0');
	  if (*f == '$')
	    ++f;
	  else
	    {
	      /* Oops; that was actually the field width.  */
	      width = argpos;
	      argpos = 0;
	      goto got_width;
	    }
	}

      /* Check for the assignment-suppressant and the number grouping flag.  */
      while (*f == '*' || *f == '\'')
	switch (*f++)
	  {
	  case '*':
#ifdef USE_IN_LIBIO
	    flags = SUPPRESS;
#endif
	    do_assign = 0;
	    break;
	  case '\'':
	    group_flag = 1;
	    break;
	  }

#ifdef USE_IN_LIBIO
      /* We have seen width. */
      if (isdigit (*f))
	flags |= WIDTH;
#endif

      /* Find the maximum field width.  */
      width = 0;
      while (isdigit (*f))
	{
	  width *= 10;
	  width += *f++ - '0';
	}
    got_width:
      if (width == 0)
	width = -1;

      /* Check for type modifiers.  */
      while (*f == 'h' || *f == 'l' || *f == 'L' || *f == 'a' || *f == 'q')
	switch (*f++)
	  {
	  case 'h':
	    /* int's are short int's.  */
#ifdef USE_IN_LIBIO
	    if (flags & ~(SUPPRESS | WIDTH))
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= SHORT;
#endif
	    is_short = 1;
	    break;
	  case 'l':
	    if (is_long)
	      {
		/* A double `l' is equivalent to an `L'.  */
#ifdef USE_IN_LIBIO
		if ((flags & ~(SUPPRESS | WIDTH)) && (flags & LONGDBL))
		  conv_error ();
		flags &= ~LONG;
		flags |= LONGDBL;
#endif
		is_longlong = 1;
	      }
	    else
	      {
		/* int's are long int's.  */
#ifdef USE_IN_LIBIO
		flags |= LONG;
#endif
		is_long = 1;
	      }
	    break;
	  case 'q':
	  case 'L':
	    /* double's are long double's, and int's are long long int's.  */
#ifdef USE_IN_LIBIO
	    if (flags & ~(SUPPRESS | WIDTH))
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= LONGDBL;
#endif
	    is_long_double = 1;
	    break;
	  case 'a':
	    /* String conversions (%s, %[) take a `char **'
	       arg and fill it in with a malloc'd pointer.  */
	    malloc_string = 1;
	    break;
	  }

      /* End of the format string?  */
      if (*f == '\0')
	conv_error ();

      /* Find the conversion specifier.  */
      w = work;
      fc = *f++;
      if (fc != '[' && fc != 'c' && fc != 'n')
	/* Eat whitespace.  */
	while (isspace (c))
	  (void) inchar ();
      switch (fc)
	{
	case '%':	/* Must match a literal '%'.  */
	  if (c != fc)
	    conv_error ();
	  break;

	case 'n':	/* Answer number of assignments done.  */
	  if (do_assign)
	    *ARG (int *) = read_in - 1;	/* Don't count the read-ahead.  */
	  break;

	case 'c':	/* Match characters.  */
	  if (do_assign)
	    {
	      str = ARG (char *);
	      if (str == NULL)
		conv_error ();
	    }

	  if (c == EOF)
	    input_error ();

	  if (width == -1)
	    width = 1;

	  if (do_assign)
	    {
	      do
		*str++ = c;
	      while (inchar () != EOF && --width > 0);
	    }
	  else
	    while (inchar () != EOF && --width > 0);

	  if (do_assign)
	    ++done;

	  break;

	case 's':		/* Read a string.  */
#define STRING_ARG							      \
	  if (do_assign)						      \
	    {								      \
	      if (malloc_string)					      \
		{							      \
		  /* The string is to be stored in a malloc'd buffer.  */     \
		  strptr = ARG (char **);				      \
		  if (strptr == NULL)					      \
		    conv_error ();					      \
		  /* Allocate an initial buffer.  */			      \
		  strsize = 100;					      \
		  *strptr = str = malloc (strsize);			      \
		}							      \
	      else							      \
		str = ARG (char *);					      \
	      if (str == NULL)						      \
		conv_error ();						      \
	    }
	  STRING_ARG;

	  if (c == EOF)
	    input_error ();

	  do
	    {
	      if (isspace (c))
		break;
#define	STRING_ADD_CHAR(c)						      \
	      if (do_assign)						      \
		{							      \
		  *str++ = c;						      \
		  if (malloc_string && str == *strptr + strsize)	      \
		    {							      \
		      /* Enlarge the buffer.  */			      \
		      str = realloc (*strptr, strsize * 2);		      \
		      if (str == NULL)					      \
			{						      \
			  /* Can't allocate that much.  Last-ditch effort.  */\
			  str = realloc (*strptr, strsize + 1);		      \
			  if (str == NULL)				      \
			    {						      \
			      /* We lose.  Oh well.			      \
				 Terminate the string and stop converting,    \
				 so at least we don't swallow any input.  */  \
			      (*strptr)[strsize] = '\0';		      \
			      ++done;					      \
			      conv_error ();				      \
			    }						      \
			  else						      \
			    {						      \
			      *strptr = str;				      \
			      str += strsize;				      \
			      ++strsize;				      \
			    }						      \
			}						      \
		      else						      \
			{						      \
			  *strptr = str;				      \
			  str += strsize;				      \
			  strsize *= 2;					      \
			}						      \
		    }							      \
		}
	      STRING_ADD_CHAR (c);
	    } while (inchar () != EOF && (width <= 0 || --width > 0));

	  if (do_assign)
	    {
	      *str = '\0';
	      ++done;
	    }
	  break;

	case 'x':	/* Hexadecimal integer.  */
	case 'X':	/* Ditto.  */
	  base = 16;
	  number_signed = 0;
	  goto number;

	case 'o':	/* Octal integer.  */
	  base = 8;
	  number_signed = 0;
	  goto number;

	case 'u':	/* Unsigned decimal integer.  */
	  base = 10;
	  number_signed = 0;
	  goto number;

	case 'd':	/* Signed decimal integer.  */
	  base = 10;
	  number_signed = 1;
	  goto number;

	case 'i':	/* Generic number.  */
	  base = 0;
	  number_signed = 1;

	number:
	  if (c == EOF)
	    input_error ();

	  /* Check for a sign.  */
	  if (c == '-' || c == '+')
	    {
	      *w++ = c;
	      if (width > 0)
		--width;
	      (void) inchar ();
	    }

	  /* Look for a leading indication of base.  */
	  if (c == '0')
	    {
	      if (width > 0)
		--width;
	      *w++ = '0';

	      (void) inchar ();

	      if (tolower (c) == 'x')
		{
		  if (base == 0)
		    base = 16;
		  if (base == 16)
		    {
		      if (width > 0)
			--width;
		      (void) inchar ();
		    }
		}
	      else if (base == 0)
		base = 8;
	    }

	  if (base == 0)
	    base = 10;

	  /* Read the number into WORK.  */
	  do
	    {
	      if (base == 16 ? !isxdigit (c) :
		  (!isdigit (c) || c - '0' >= base))
		break;
	      *w++ = c;
	      if (width > 0)
		--width;
	    }
	  while (inchar () != EOF && width != 0);

	  if (w == work ||
	      (w - work == 1 && (work[0] == '+' || work[0] == '-')))
	    /* There was no number.  */
	    conv_error ();

	  /* Convert the number.  */
	  *w = '\0';
	  if (is_longlong)
	    {
	      if (number_signed)
		num.q = __strtoq_internal (work, &w, base, group_flag);
	      else
		num.uq = __strtouq_internal (work, &w, base, group_flag);
	    }
	  else
	    {
	      if (number_signed)
		num.l = __strtol_internal (work, &w, base, group_flag);
	      else
		num.ul = __strtoul_internal (work, &w, base, group_flag);
	    }
	  if (w == work)
	    conv_error ();

	  if (do_assign)
	    {
	      if (! number_signed)
		{
		  if (is_longlong)
		    *ARG (unsigned LONGLONG int *) = num.uq;
		  else if (is_long)
		    *ARG (unsigned long int *) = num.ul;
		  else if (is_short)
		    *ARG (unsigned short int *)
		      = (unsigned short int) num.ul;
		  else
		    *ARG (unsigned int *) = (unsigned int) num.ul;
		}
	      else
		{
		  if (is_longlong)
		    *ARG (LONGLONG int *) = num.q;
		  else if (is_long)
		    *ARG (long int *) = num.l;
		  else if (is_short)
		    *ARG (short int *) = (short int) num.l;
		  else
		    *ARG (int *) = (int) num.l;
		}
	      ++done;
	    }
	  break;

	case 'e':	/* Floating-point numbers.  */
	case 'E':
	case 'f':
	case 'g':
	case 'G':
	  if (c == EOF)
	    input_error ();

	  /* Check for a sign.  */
	  if (c == '-' || c == '+')
	    {
	      *w++ = c;
	      if (inchar () == EOF)
		/* EOF is only an input error before we read any chars.  */
		conv_error ();
	      if (width > 0)
		--width;
	    }

	  got_dot = got_e = 0;
	  do
	    {
	      if (isdigit (c))
		*w++ = c;
	      else if (got_e && w[-1] == 'e' && (c == '-' || c == '+'))
		*w++ = c;
	      else if (!got_e && tolower (c) == 'e')
		{
		  *w++ = 'e';
		  got_e = got_dot = 1;
		}
	      else if (c == decimal && !got_dot)
		{
		  *w++ = c;
		  got_dot = 1;
		}
	      else
		break;
	      if (width > 0)
		--width;
	    } while (inchar () != EOF && width != 0);

	  if (w == work)
	    conv_error();
	  if (w[-1] == '-' || w[-1] == '+' || w[-1] == 'e')
	    conv_error ();

	  /* Convert the number.  */
	  *w = '\0';
	  if (is_long_double)
	    {
	      long double d = __strtold_internal (work, &w, group_flag);
	      if (do_assign && w != work)
		*ARG (long double *) = d;
	    }
	  else if (is_long)
	    {
	      double d = __strtod_internal (work, &w, group_flag);
	      if (do_assign && w != work)
		*ARG (double *) = d;
	    }
	  else
	    {
	      float d = __strtof_internal (work, &w, group_flag);
	      if (do_assign && w != work)
		*ARG (float *) = d;
	    }

	  if (w == work)
	    conv_error ();

	  if (do_assign)
	    ++done;
	  break;

	case '[':	/* Character class.  */
	  STRING_ARG;

	  if (c == EOF)
	    input_error();

	  if (*f == '^')
	    {
	      ++f;
	      not_in = 1;
	    }
	  else
	    not_in = 0;

	  while ((fc = *f++) != '\0' && fc != ']')
	    {
	      if (fc == '-' && *f != '\0' && *f != ']' &&
		  w > work && w[-1] <= *f)
		/* Add all characters from the one before the '-'
		   up to (but not including) the next format char.  */
		for (fc = w[-1] + 1; fc < *f; ++fc)
		  *w++ = fc;
	      else
		/* Add the character to the list.  */
		*w++ = fc;
	    }
	  if (fc == '\0')
	    conv_error();

	  *w = '\0';
	  num.ul = read_in;
	  do
	    {
	      if ((strchr (work, c) == NULL) != not_in)
		break;
	      STRING_ADD_CHAR (c);
	      if (width > 0)
		--width;
	    } while (inchar () != EOF && width != 0);
	  if (read_in == num.ul)
	    conv_error ();

	  if (do_assign)
	    {
	      *str = '\0';
	      ++done;
	    }
	  break;

	case 'p':	/* Generic pointer.  */
	  base = 16;
	  /* A PTR must be the same size as a `long int'.  */
	  is_long = 1;
	  number_signed = 0;
	  goto number;
	}
    }

  return ((c == EOF || ungetc (c, s)), done);
}

#ifdef USE_IN_LIBIO
int
__vfscanf (FILE *s, const char *format, va_list argptr)
{
  return _IO_vfscanf (s, format, argptr, NULL);
}
#endif

weak_alias (__vfscanf, vfscanf)
