/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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

/* Those are flags in the conversion format. */
# define LONG		0x001	/* l: long or double */
# define LONGDBL	0x002	/* L: long long or long double */
# define SHORT		0x004	/* h: short */
# define SUPPRESS	0x008	/* *: suppress assignment */
# define POINTER	0x010	/* weird %p pointer (`fake hex') */
# define NOSKIP		0x020	/* do not skip blanks */
# define WIDTH		0x040	/* width was given */
# define GROUP		0x080	/* ': group numbers */
# define MALLOC		0x100	/* a: malloc strings */


#ifdef USE_IN_LIBIO
# include <libioP.h>
# include <libio.h>

# define va_list	_IO_va_list
# define ungetc(c, s)	_IO_ungetc (c, s)
# define inchar()	((c = _IO_getc (s)), (void) ++read_in, c)
# define conv_error()	return ((void) (errp != NULL && (*errp |= 2)), \
				(void) (c == EOF || _IO_ungetc (c, s)), done)

# define input_error()	return ((void) (errp != NULL && (*errp |= 1)), \
				done == 0 ? EOF : done)
# define memory_error()	return ((void) (errno = ENOMEM), EOF)
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
# define inchar()	((c = getc (s)), (void) ++read_in, c)
# define conv_error()	return ((void) ungetc (c, s), done)
# define input_error()	return (done == 0 ? EOF : done)
# define memory_error()	return ((void) (errno = ENOMEM), EOF)
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
  register unsigned char fc;	/* Current character of the format.  */
  register size_t done = 0;	/* Assignments done.  */
  register size_t read_in = 0;	/* Chars read in.  */
  register int c;		/* Last char read.  */
  register int width;		/* Maximum field width.  */
  register int flags;		/* Modifiers for current format element.  */

  /* Status for reading F-P nums.  */
  char got_dot, got_e;
  /* If a [...] is a [^...].  */
  char not_in;
  /* Base for integral numbers.  */
  int base;
  /* Signedness for integral numbers.  */
  int number_signed;
  /* Decimal point character.  */
  wchar_t decimal;
  /* The thousands character of the current locale.  */
  wchar_t thousands;
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
  /* We must not react on white spaces immediately because they can
     possibly be matched even if in the input stream no character is
     available anymore.  */
  int skip_space = 0;
  /* Workspace.  */
  char *tw;			/* Temporary pointer.  */
  char *wp = NULL;		/* Workspace.  */
  size_t wpmax = 0;		/* Maximal size of workspace.  */
  size_t wpsize;		/* Currently used bytes in workspace.  */
#define ADDW(Ch)							    \
  do									    \
    {									    \
      if (wpsize == wpmax)						    \
	{								    \
	  char *old = wp;						    \
	  wpmax = UCHAR_MAX > 2 * wpmax ? UCHAR_MAX : 2 * wpmax;	    \
	  wp = (char *) alloca (wpmax);					    \
	  if (old != NULL)						    \
	    memcpy (wp, old, wpsize);					    \
	}								    \
      wp[wpsize++] = (Ch);						    \
    }									    \
  while (0)

  ARGCHECK (s, format);

  /* Figure out the decimal point character.  */
  if (mbtowc (&decimal, _NL_CURRENT (LC_NUMERIC, DECIMAL_POINT),
	      strlen (_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT))) <= 0)
    decimal = (wchar_t) *_NL_CURRENT (LC_NUMERIC, DECIMAL_POINT);
  /* Figure out the thousands separator character.  */
  if (mbtowc (&thousands, _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP),
	      strlen (_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP))) <= 0)
    thousands = (wchar_t) *_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);

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
	  /* Remember to skip spaces.  */
	  if (isspace (fc))
	    {
	      skip_space = 1;
	      continue;
	    }

	  /* Characters other than format specs must just match.  */
	  if (c == EOF)
	    input_error ();

	  /* We saw white space char as the last character in the format
	     string.  Now it's time to skip all leading white space.  */
	  if (skip_space)
	    {
	      while (isspace (c))
		(void) inchar ();
	      skip_space = 0;
	    }

	  if (c == fc)
	    (void) inchar ();
	  else
	    conv_error ();

	  continue;
	}

      /* This is the start of the conversion string. */
      flags = 0;

      /* Initialize state of modifiers.  */
      argpos = 0;

      /* Prepare temporary buffer.  */
      wpsize = 0;

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
	      flags |= WIDTH;
	      argpos = 0;
	      goto got_width;
	    }
	}

      /* Check for the assignment-suppressant and the number grouping flag.  */
      while (*f == '*' || *f == '\'')
	switch (*f++)
	  {
	  case '*':
	    flags |= SUPPRESS;
	    break;
	  case '\'':
	    flags |= GROUP;
	    break;
	  }

      /* We have seen width. */
      if (isdigit (*f))
	flags |= WIDTH;

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
	    if (flags & (LONG|LONGDBL))
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= SHORT;
	    break;
	  case 'l':
	    if (flags & SHORT)
	      conv_error ();
	    else if (flags & LONG)
	      {
		/* A double `l' is equivalent to an `L'.  */
		flags &= ~LONG;
		flags |= LONGDBL;
	      }
	    else
	      /* int's are long int's.  */
	      flags |= LONG;
	    break;
	  case 'q':
	  case 'L':
	    /* double's are long double's, and int's are long long int's.  */
	    if (flags & (LONG|SHORT))
	      /* Signal illegal format element.  */
	      conv_error ();
	    flags |= LONGDBL;
	    break;
	  case 'a':
	    /* String conversions (%s, %[) take a `char **'
	       arg and fill it in with a malloc'd pointer.  */
	    flags |= MALLOC;
	    break;
	  }

      /* End of the format string?  */
      if (*f == '\0')
	conv_error ();

      /* Find the conversion specifier.  */
      fc = *f++;
      if (skip_space || (fc != '[' && fc != 'c' && fc != 'n'))
	{
	  /* Eat whitespace.  */
	  while (isspace (c))
	    (void) inchar ();
	  skip_space = 0;
	}

      switch (fc)
	{
	case '%':	/* Must match a literal '%'.  */
	  if (c != fc)
	    conv_error ();
	  inchar ();
	  break;

	case 'n':	/* Answer number of assignments done.  */
	  if (!(flags & SUPPRESS))
	    *ARG (int *) = read_in - 1;	/* Don't count the read-ahead.  */
	  break;

	case 'c':	/* Match characters.  */
	  if (!(flags & SUPPRESS))
	    {
	      str = ARG (char *);
	      if (str == NULL)
		conv_error ();
	    }

	  if (c == EOF)
	    input_error ();

	  if (width == -1)
	    width = 1;

	  if (!(flags & SUPPRESS))
	    {
	      do
		*str++ = c;
	      while (inchar () != EOF && --width > 0);
	    }
	  else
	    while (inchar () != EOF && --width > 0);

	  if (!(flags & SUPPRESS))
	    ++done;

	  break;

	case 's':		/* Read a string.  */
#define STRING_ARG							      \
	  if (!(flags & SUPPRESS))					      \
	    {								      \
	      if (flags & MALLOC)					      \
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
	      if (!(flags & SUPPRESS))					      \
		{							      \
		  *str++ = c;						      \
		  if ((flags & MALLOC) && str == *strptr + strsize)	      \
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
				 so at least we don't skip any input.  */  \
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

	  if (!(flags & SUPPRESS))
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
	      ADDW (c);
	      if (width > 0)
		--width;
	      (void) inchar ();
	    }

	  /* Look for a leading indication of base.  */
	  if (width != 0 && c == '0')
	    {
	      if (width > 0)
		--width;
	      ADDW ('0');

	      (void) inchar ();

	      if (width != 0 && tolower (c) == 'x')
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

	  /* Read the number into workspace.  */
	  while (c != EOF && width != 0)
	    {
	      if (base == 16 ? !isxdigit (c) :
		  ((!isdigit (c) || c - '0' >= base) &&
		   !((flags & GROUP) && base == 10 && c == thousands)))
		break;
	      ADDW (c);
	      if (width > 0)
		--width;

	      (void) inchar ();
	    }

	  if (wpsize == 0 ||
	      (wpsize == 1 && (wp[0] == '+' || wp[0] == '-')))
	    /* There was no number.  */
	    conv_error ();

	  /* Convert the number.  */
	  ADDW ('\0');
	  if (flags & LONGDBL)
	    {
	      if (number_signed)
		num.q = __strtoq_internal (wp, &tw, base, flags & GROUP);
	      else
		num.uq = __strtouq_internal (wp, &tw, base, flags & GROUP);
	    }
	  else
	    {
	      if (number_signed)
		num.l = __strtol_internal (wp, &tw, base, flags & GROUP);
	      else
		num.ul = __strtoul_internal (wp, &tw, base, flags & GROUP);
	    }
	  if (wp == tw)
	    conv_error ();

	  if (!(flags & SUPPRESS))
	    {
	      if (! number_signed)
		{
		  if (flags & LONGDBL)
		    *ARG (unsigned LONGLONG int *) = num.uq;
		  else if (flags & LONG)
		    *ARG (unsigned long int *) = num.ul;
		  else if (flags & SHORT)
		    *ARG (unsigned short int *)
		      = (unsigned short int) num.ul;
		  else
		    *ARG (unsigned int *) = (unsigned int) num.ul;
		}
	      else
		{
		  if (flags & LONGDBL)
		    *ARG (LONGLONG int *) = num.q;
		  else if (flags & LONG)
		    *ARG (long int *) = num.l;
		  else if (flags & SHORT)
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
	      ADDW (c);
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
		ADDW (c);
	      else if (got_e && wp[wpsize - 1] == 'e'
		       && (c == '-' || c == '+'))
		ADDW (c);
	      else if (!got_e && tolower (c) == 'e')
		{
		  ADDW ('e');
		  got_e = got_dot = 1;
		}
	      else if (c == decimal && !got_dot)
		{
		  ADDW (c);
		  got_dot = 1;
		}
	      else if ((flags & GROUP) && c == thousands && !got_dot)
		ADDW (c);
	      else
		break;
	      if (width > 0)
		--width;
	    } while (inchar () != EOF && width != 0);

	  if (wpsize == 0)
	    conv_error();
	  if (wp[wpsize - 1] == '-' || wp[wpsize - 1] == '+'
	      || wp[wpsize - 1] == 'e')
	    conv_error ();

	  /* Convert the number.  */
	  ADDW ('\0');
	  if (flags & LONGDBL)
	    {
	      long double d = __strtold_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (long double *) = d;
	    }
	  else if (flags & LONG)
	    {
	      double d = __strtod_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (double *) = d;
	    }
	  else
	    {
	      float d = __strtof_internal (wp, &tw, flags & GROUP);
	      if (!(flags & SUPPRESS) && tw != wp)
		*ARG (float *) = d;
	    }

	  if (tw == wp)
	    conv_error ();

	  if (!(flags & SUPPRESS))
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

	  /* Fill WP with byte flags indexed by character.
	     We will use this flag map for matching input characters.  */
	  if (wpmax < UCHAR_MAX)
	    {
	      wpmax = UCHAR_MAX;
	      wp = (char *) alloca (wpmax);
	    }
	  memset (wp, 0, UCHAR_MAX);

	  fc = *f;
	  if (fc == ']' || fc == '-')
	    {
	      /* If ] or - appears before any char in the set, it is not
		 the terminator or separator, but the first char in the
		 set.  */
	      wp[fc] = 1;
	      ++f;
	    }

	  while ((fc = *f++) != '\0' && fc != ']')
	    {
	      if (fc == '-' && *f != '\0' && *f != ']' &&
		  (unsigned char) f[-2] <= (unsigned char) *f)
		{
		  /* Add all characters from the one before the '-'
		     up to (but not including) the next format char.  */
		  for (fc = f[-2]; fc < *f; ++fc)
		    wp[fc] = 1;
		}
	      else
		/* Add the character to the flag map.  */
		wp[fc] = 1;
	    }
	  if (fc == '\0')
	    conv_error();

	  num.ul = read_in;
	  do
	    {
	      if (wp[c] == not_in)
		break;
	      STRING_ADD_CHAR (c);
	      if (width > 0)
		--width;
	    } while (inchar () != EOF && width != 0);
	  if (read_in == num.ul)
	    conv_error ();

	  if (!(flags & SUPPRESS))
	    {
	      *str = '\0';
	      ++done;
	    }
	  break;

	case 'p':	/* Generic pointer.  */
	  base = 16;
	  /* A PTR must be the same size as a `long int'.  */
	  flags &= ~(SHORT|LONGDBL);
	  flags |= LONG;
	  number_signed = 0;
	  goto number;
	}
    }

  /* The last thing we saw int the format string was a white space.
     Consume the last white spaces.  */
  if (skip_space)
    while (isspace (c))
      (void) inchar ();

  return ((void) (c == EOF || ungetc (c, s)), done);
}

#ifdef USE_IN_LIBIO
int
__vfscanf (FILE *s, const char *format, va_list argptr)
{
  return _IO_vfscanf (s, format, argptr, NULL);
}
#endif

weak_alias (__vfscanf, vfscanf)
