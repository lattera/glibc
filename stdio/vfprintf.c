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
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <printf.h>
#include <assert.h>
#include <stddef.h>
#include "_itoa.h"

/* This function from the GNU C library is also used in libio.
   To compile for use in libio, compile with -DUSE_IN_LIBIO.  */

#ifdef USE_IN_LIBIO
/* This code is for use in libio.  */
#include <libioP.h>
#define PUT(f, s, n)	_IO_sputn (f, s, n)
#define PAD(padchar)	\
	(width > 0 ? width += _IO_padn (s, padchar, width) : 0)
#define PUTC(c, f)	_IO_putc(c, f)
#define vfprintf	_IO_vfprintf
#define size_t		_IO_size_t
#define FILE		_IO_FILE
#define va_list		_IO_va_list
#undef	BUFSIZ
#define BUFSIZ		_IO_BUFSIZ
#define ARGCHECK(s, format) \
  do									      \
    {									      \
      /* Check file argument for consistence.  */			      \
      CHECK_FILE(s, -1);						      \
      if (s->_flags & _IO_NO_WRITES || format == NULL)			      \
	{								      \
	  MAYBE_SET_EINVAL;						      \
	  return -1;							      \
	}								      \
    } while (0)
#define UNBUFFERED_P(s)	((s)->_IO_file_flags & _IO_UNBUFFERED)
#else /* ! USE_IN_LIBIO */
/* This code is for use in the GNU C library.  */
#include <stdio.h>
#define PUTC(c, f)	putc (c, f)
#define PUT(f, s, n)	fwrite (s, 1, n, f)
ssize_t __printf_pad __P ((FILE *, char pad, int n));
#define PAD(padchar)	__printf_pad (s, padchar, width)
#define ARGCHECK(s, format) \
  do									      \
    {									      \
      /* Check file argument for consistence.  */			      \
      if (!__validfp(s) || !s->__mode.__write || format == NULL)	      \
	{								      \
	  errno = EINVAL;						      \
	  return -1;							      \
	}								      \
      if (!s->__seen)							      \
	{								      \
	  if (__flshfp (s, EOF) == EOF)					      \
	    return -1;							      \
	}								      \
    } while (0)
#define UNBUFFERED_P(s)	((s)->__buffer == NULL)
#endif /* USE_IN_LIBIO */


#define	outchar(x)							      \
  do									      \
    {									      \
      register CONST int outc = (x);					      \
      if (putc(outc, s) == EOF)						      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

/* Advances STRING after writing LEN chars of it.  */
#define outstring(string, len)						      \
  do									      \
    {									      \
      if (len > 20)							      \
	{								      \
	  if (PUT (s, string, len) != len)				      \
	    return -1;							      \
	  done += len;							      \
	  string += len;						      \
	}								      \
      else								      \
	while (len-- > 0)						      \
	  outchar (*string++);						      \
    } while (0)

/* Helper function to provide temporary buffering for unbuffered streams.  */
static int buffered_vfprintf __P ((FILE *stream, const char *fmt, va_list));

/* Cast the next arg, of type ARGTYPE, into CASTTYPE, and put it in VAR.  */
#define	castarg(var, argtype, casttype) \
  var = (casttype) va_arg(args, argtype)
/* Get the next arg, of type TYPE, and put it in VAR.  */
#define	nextarg(var, type)	castarg(var, type, type)

static printf_function printf_unknown;

extern printf_function **__printf_function_table;

#ifdef	__GNUC__
#define	HAVE_LONGLONG
#define	LONGLONG	long long
#else
#define	LONGLONG	long
#endif

static char *group_number __P ((char *, char *, const char *, wchar_t));

int
DEFUN(vfprintf, (s, format, args),
      register FILE *s AND CONST char *format AND va_list args)
{
  /* The character used as thousands separator.  */
  wchar_t thousands_sep;

  /* The string describing the size of groups of digits.  */
  const char *grouping; 

  /* Pointer into the format string.  */
  register CONST char *f;

  /* Number of characters written.  */
  register size_t done = 0;

  ARGCHECK (s, format);

  if (UNBUFFERED_P (s))
    /* Use a helper function which will allocate a local temporary buffer
       for the stream and then call us again.  */
    return buffered_vfprintf (s, format, args);

  /* Reset multibyte characters to their initial state.  */
  (void) mblen ((char *) NULL, 0);

  /* Figure out the thousands seperator character.  */
  if (mbtowc (&thousands_sep, _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP),
	      strlen (_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP))) <= 0)
    thousands_sep = (wchar_t) *_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);
  grouping = _NL_CURRENT (LC_NUMERIC, GROUPING);
  if (*grouping == '\0' || thousands_sep == L'\0')
    grouping = NULL;

  f = format;
  while (*f != '\0')
    {
      /* Type modifiers.  */
      char is_short, is_long, is_long_double;
#ifdef	HAVE_LONGLONG
      /* We use the `L' modifier for `long long int'.  */
#define	is_longlong	is_long_double
#else
#define	is_longlong	0
#endif
      /* Format spec modifiers.  */
      char space, showsign, left, alt, group;

      /* Padding character: ' ' or '0'.  */
      char pad;
      /* Width of a field.  */
      register int width;
      /* Precision of a field.  */
      int prec;

      /* Decimal integer is negative.  */
      char is_neg;

      /* Current character of the format.  */
      char fc;

      /* Base of a number to be written.  */
      int base;
      /* Integral values to be written.  */
      unsigned LONGLONG int num;
      LONGLONG int signed_num;

      /* String to be written.  */
      CONST char *str;
      char errorbuf[1024];	/* Buffer sometimes used by %m.  */

      /* Auxiliary function to do output.  */
      printf_function *function;

      if (!isascii(*f))
	{
	  /* Non-ASCII, may be a multibyte.  */
	  int len = mblen (f, strlen (f));
	  if (len > 0)
	    {
	      outstring (f, len);
	      continue;
	    }
	}

      if (*f != '%')
	{
	  /* This isn't a format spec, so write everything out until the
	     next one.  To properly handle multibyte characters, we cannot
	     just search for a '%'.  Since multibyte characters are hairy
	     (and dealt with above), if we hit any byte above 127 (only
	     those can start a multibyte character) we just punt back to
	     that code.  */
	  do
	    outchar (*f++);
	  while (*f != '\0' && *f != '%' && isascii (*f));
	  continue;
	}

      ++f;

      /* Check for "%%".  Note that although the ANSI standard lists
	 '%' as a conversion specifier, it says "The complete format
	 specification shall be `%%'," so we can avoid all the width
	 and precision processing.  */
      if (*f == '%')
	{
	  ++f;
	  outchar('%');
	  continue;
	}

      /* Check for spec modifiers.  */
      space = showsign = left = alt = group = 0;
      pad = ' ';
      while (*f == ' ' || *f == '+' || *f == '-' || *f == '#' || *f == '0' ||
	     *f == '\'')
	switch (*f++)
	  {
	  case ' ':
	    /* Output a space in place of a sign, when there is no sign.  */
	    space = 1;
	    break;
	  case '+':
	    /* Always output + or - for numbers.  */
	    showsign = 1;
	    break;
	  case '-':
	    /* Left-justify things.  */
	    left = 1;
	    break;
	  case '#':
	    /* Use the "alternate form":
	       Hex has 0x or 0X, FP always has a decimal point.  */
	    alt = 1;
	    break;
	  case '0':
	    /* Pad with 0s.  */
	    pad = '0';
	    break;
	  case '\'':
	    /* Show grouping in numbers if the locale information
	       indicates any.  */
	    group = 1;
	    break;
	  }
      if (left)
	pad = ' ';

      /* Get the field width.  */
      width = 0;
      if (*f == '*')
	{
	  /* The field width is given in an argument.
	     A negative field width indicates left justification.  */
	  nextarg(width, int);
	  if (width < 0)
	    {
	      width = - width;
	      left = 1;
	    }
	  ++f;
	}
      else
	while (isdigit (*f))
	  {
	    width *= 10;
	    width += *f++ - '0';
	  }

      /* Get the precision.  */
      /* -1 means none given; 0 means explicit 0.  */
      prec = -1;
      if (*f == '.')
	{
	  ++f;
	  if (*f == '*')
	    {
	      /* The precision is given in an argument.  */
	      nextarg(prec, int);
	      /* Avoid idiocy.  */
	      if (prec < 0)
		prec = -1;
	      ++f;
	    }
	  else if (isdigit (*f))
	    {
	      prec = *f++ - '0';
	      while (*f != '\0' && isdigit (*f))
		{
		  prec *= 10;
		  prec += *f++ - '0';
		}
	    }
	  else
	    /* "%.?" is treated like "%.0?".  */
	    prec = 0;
	}

      /* If there was a precision specified, ignore the 0 flag and always
	 pad with spaces.  */
      if (prec != -1)
	pad = ' ';

      /* Check for type modifiers.  */
      is_short = is_long = is_long_double = 0;
      while (*f == 'h' || *f == 'l' || *f == 'L' || *f == 'q' || *f == 'Z')
	switch (*f++)
	  {
	  case 'h':
	    /* int's are short int's.  */
	    is_short = 1;
	    break;
	  case 'l':
#ifdef	HAVE_LONGLONG
	    if (is_long)
	      /* A double `l' is equivalent to an `L'.  */
	      is_longlong = 1;
	    else
#endif
	      /* int's are long int's.  */
	      is_long = 1;
	    break;
	  case 'L':
	    /* double's are long double's, and int's are long long int's.  */
	    is_long_double = 1;
	    break;

	  case 'Z':
	    /* int's are size_t's.  */
#ifdef	HAVE_LONGLONG
	    assert (sizeof(size_t) <= sizeof(unsigned long long int));
	    is_longlong = sizeof(size_t) > sizeof(unsigned long int);
#endif
	    is_long = sizeof(size_t) > sizeof(unsigned int);
	    break;

	  case 'q':
	    /* 4.4 uses this for long long.  */
#ifdef	HAVE_LONGLONG
	    is_longlong = 1;
#else
	    is_long = 1;
#endif
	    break;
	  }

      /* Format specification.  */
      fc = *f++;
      function = (__printf_function_table == NULL ? NULL :
		  __printf_function_table[fc]);
      if (function == NULL)
	switch (fc)
	  {
	  case 'i':
	  case 'd':
	    /* Decimal integer.  */
	    base = 10;
	    if (is_longlong)
	      nextarg(signed_num, LONGLONG int);
	    else if (is_long)
	      nextarg(signed_num, long int);
	    else if (!is_short)
	      castarg(signed_num, int, long int);
	    else
	      castarg(signed_num, int, short int);

	    is_neg = signed_num < 0;
	    num = is_neg ? (- signed_num) : signed_num;
	    goto number;

	  case 'u':
	    /* Decimal unsigned integer.  */
	    base = 10;
	    goto unsigned_number;

	  case 'o':
	    /* Octal unsigned integer.  */
	    base = 8;
	    goto unsigned_number;

	  case 'X':
	    /* Hexadecimal unsigned integer.  */
	  case 'x':
	    /* Hex with lower-case digits.  */

	    base = 16;

	  unsigned_number:
	    /* Unsigned number of base BASE.  */

	    if (is_longlong)
	      castarg(num, LONGLONG int, unsigned LONGLONG int);
	    else if (is_long)
	      castarg(num, long int, unsigned long int);
	    else if (!is_short)
	      castarg(num, int, unsigned int);
	    else
	      castarg(num, int, unsigned short int);

	    /* ANSI only specifies the `+' and
	       ` ' flags for signed conversions.  */
	    is_neg = showsign = space = 0;

	  number:
	    /* Number of base BASE.  */
	    {
	      char work[BUFSIZ];
	      char *CONST workend = &work[sizeof(work) - 1];
	      register char *w;

	      /* Supply a default precision if none was given.  */
	      if (prec == -1)
		prec = 1;

	      /* Put the number in WORK.  */
	      w = _itoa (num, workend + 1, base, fc == 'X') - 1;
	      if (group && grouping)
		w = group_number (w, workend, grouping, thousands_sep);
	      width -= workend - w;
	      prec -= workend - w;

	      if (alt && base == 8 && prec <= 0)
		{
		  *w-- = '0';
		  --width;
		}

	      if (prec > 0)
		{
		  width -= prec;
		  while (prec-- > 0)
		    *w-- = '0';
		}

	      if (alt && base == 16)
		width -= 2;

	      if (is_neg || showsign || space)
		--width;

	      if (!left && pad == ' ')
		PAD (' ');

	      if (is_neg)
		outchar('-');
	      else if (showsign)
		outchar('+');
	      else if (space)
		outchar(' ');

	      if (alt && base == 16)
		{
		  outchar ('0');
		  outchar (fc);
		}

	      if (!left && pad == '0')
		PAD ('0');

	      /* Write the number.  */
	      while (++w <= workend)
		outchar(*w);

	      if (left)
		PAD (' ');
	    }
	    break;

	  case 'e':
	  case 'E':
	  case 'f':
	  case 'g':
	  case 'G':
	    {
	      /* Floating-point number.  */
	      extern printf_function __printf_fp;
	      function = __printf_fp;
	      goto use_function;
	    }

	  case 'c':
	    /* Character.  */
	    nextarg(num, int);
	    if (!left)
	      {
		--width;
		PAD (' ');
	      }
	    outchar ((unsigned char) num);
	    if (left)
	      PAD (' ');
	    break;

	  case 's':
	    {
	      static CONST char null[] = "(null)";
	      size_t len;

	      nextarg(str, CONST char *);

	    string:

	      if (str == NULL)
		/* Write "(null)" if there's space.  */
		if (prec == -1 || prec >= (int) sizeof(null) - 1)
		  {
		    str = null;
		    len = sizeof(null) - 1;
		  }
		else
		  {
		    str = "";
		    len = 0;
		  }
	      else
		len = strlen(str);

	      if (prec != -1 && (size_t) prec < len)
		len = prec;
	      width -= len;

	      if (!left)
		PAD (' ');
	      outstring (str, len);
	      if (left)
		PAD (' ');
	    }
	    break;

	  case 'p':
	    /* Generic pointer.  */
	    {
	      CONST PTR ptr;
	      nextarg(ptr, CONST PTR);
	      if (ptr != NULL)
		{
		  /* If the pointer is not NULL, write it as a %#x spec.  */
		  base = 16;
		  fc = 'x';
		  alt = 1;
		  num = (unsigned LONGLONG int) (unsigned long int) ptr;
		  is_neg = 0;
		  group = 0;
		  goto number;
		}
	      else
		{
		  /* Write "(nil)" for a nil pointer.  */
		  static CONST char nil[] = "(nil)";
		  register CONST char *p;

		  width -= sizeof (nil) - 1;
		  if (!left)
		    PAD (' ');
		  for (p = nil; *p != '\0'; ++p)
		    outchar (*p);
		  if (left)
		    PAD (' ');
		}
	    }
	    break;

	  case 'n':
	    /* Answer the count of characters written.  */
	    if (is_longlong)
	      {
		LONGLONG int *p;
		nextarg(p, LONGLONG int *);
		*p = done;
	      }
	    else if (is_long)
	      {
		long int *p;
		nextarg(p, long int *);
		*p = done;
	      }
	    else if (!is_short)
	      {
		int *p;
		nextarg(p, int *);
		*p = done;
	      }
	    else
	      {
		short int *p;
		nextarg(p, short int *);
		*p = done;
	      }
	    break;

	  case 'm':
	    {
	      extern char *_strerror_internal __P ((int, char buf[1024]));
	      str = _strerror_internal (errno, errorbuf);
	      goto string;
	    }

	  default:
	    /* Unrecognized format specifier.  */
	    function = printf_unknown;
	    goto use_function;
	  }
      else
      use_function:
	{
	  int function_done;
	  struct printf_info info;

	  info.prec = prec;
	  info.width = width;
	  info.spec = fc;
	  info.is_long_double = is_long_double;
	  info.is_short = is_short;
	  info.is_long = is_long;
	  info.alt = alt;
	  info.space = space;
	  info.left = left;
	  info.showsign = showsign;
	  info.group = group;
	  info.pad = pad;

	  function_done = (*function) (s, &info, &args);
	  if (function_done < 0)
	    return -1;

	  done += function_done;
	}
    }

  return done;
}


static int
DEFUN(printf_unknown, (s, info, arg),
      FILE *s AND CONST struct printf_info *info AND va_list *arg)
{
  int done = 0;
  char work[BUFSIZ];
  char *CONST workend = &work[sizeof(work) - 1];
  register char *w;
  register int prec = info->prec, width = info->width;

  outchar('%');

  if (info->alt)
    outchar ('#');
  if (info->group)
    outchar ('\'');
  if (info->showsign)
    outchar ('+');
  else if (info->space)
    outchar (' ');
  if (info->left)
    outchar ('-');
  if (info->pad == '0')
    outchar ('0');

  w = workend;
  while (width > 0)
    {
      *w-- = '0' + (width % 10);
      width /= 10;
    }
  while (++w <= workend)
    outchar(*w);

  if (info->prec != -1)
    {
      outchar('.');
      w = workend;
      while (prec > 0)
	{
	  *w-- = '0' + (prec % 10);
	  prec /= 10;
	}
      while (++w <= workend)
	outchar(*w);
    }

  outchar(info->spec);

  return done;
}

/* Group the digits according to the grouping rules of the current locale.
   The interpretation of GROUPING is as in `struct lconv' from <locale.h>.  */

static char *
group_number (char *w, char *workend, const char *grouping,
	      wchar_t thousands_sep)
{
  int len;
  char *src, *s;

  /* We treat all negative values like CHAR_MAX.  */

  if (*grouping == CHAR_MAX || *grouping < 0)
    /* No grouping should be done.  */
    return w;

  len = *grouping;

  /* Copy existing string so that nothing gets overwritten.  */
  src = (char *) alloca (workend - w);
  memcpy (src, w + 1, workend - w);
  s = &src[workend - w - 1];
  w = workend;

  /* Process all characters in the string.  */
  while (s >= src)
    {
      *w-- = *s--;

      if (--len == 0 && s >= src)
	{
	  /* A new group begins.  */
	  *w-- = thousands_sep;

	  len = *grouping++;
	  if (*grouping == '\0')
	    /* The previous grouping repeats ad infinitum.  */
	    --grouping;
	  else if (*grouping == CHAR_MAX || *grouping < 0)
	    {
	      /* No further grouping to be done.
		 Copy the rest of the number.  */
	      do
		*w-- = *s--;
	      while (s >= src);
	      break;
	    }
	}
    }

  return w;
}

#ifdef USE_IN_LIBIO
/* Helper "class" for `fprintf to unbuffered': creates a temporary buffer.  */
struct helper_file
  {
    struct _IO_FILE_plus _f;
    _IO_FILE *_put_stream;
  };

static int
DEFUN(_IO_helper_overflow, (s, c), _IO_FILE *s AND int c)
{
  _IO_FILE *target = ((struct helper_file*) s)->_put_stream;
  int used = s->_IO_write_ptr - s->_IO_write_base;
  if (used)
    {
      _IO_size_t written = _IO_sputn (target, s->_IO_write_base, used);
      s->_IO_write_ptr -= written;
    }
  return _IO_putc (c, s);
}

static const struct _IO_jump_t _IO_helper_jumps =
  {
    _IO_helper_overflow,
    _IO_default_underflow,
    _IO_default_xsputn,
    _IO_default_xsgetn,
    _IO_default_read,
    _IO_default_write,
    _IO_default_doallocate,
    _IO_default_pbackfail,
    _IO_default_setbuf,
    _IO_default_sync,
    _IO_default_finish,
    _IO_default_close,
    _IO_default_stat,
    _IO_default_seek,
    _IO_default_seekoff,
    _IO_default_seekpos,
    _IO_default_uflow
  };

static int
DEFUN(buffered_vfprintf, (s, format, args),
      register _IO_FILE *s AND char CONST *format AND _IO_va_list args)
{
  char buf[_IO_BUFSIZ];
  struct helper_file helper;
  register _IO_FILE *hp = (_IO_FILE *) &helper;
  int result, to_flush;

  /* Initialize helper.  */
  helper._put_stream = s;
  hp->_IO_write_base = buf;
  hp->_IO_write_ptr = buf;
  hp->_IO_write_end = buf + sizeof buf;
  hp->_IO_file_flags = _IO_MAGIC|_IO_NO_READS;
  hp->_jumps = (struct _IO_jump_t *) &_IO_helper_jumps;
  
  /* Now print to helper instead.  */
  result = _IO_vfprintf (hp, format, args);

  /* Now flush anything from the helper to the S. */
  if ((to_flush = hp->_IO_write_ptr - hp->_IO_write_base) > 0)
    {
      if (_IO_sputn (s, hp->_IO_write_base, to_flush) != to_flush)
	return -1;
    }

  return result;
}

#else /* !USE_IN_LIBIO */

static int
DEFUN(buffered_vfprintf, (s, format, args),
      register FILE *s AND char CONST *format AND va_list args)
{
  char buf[BUFSIZ];
  int result;

  s->__bufp = s->__buffer = buf;
  s->__bufsize = sizeof buf;
  s->__put_limit = s->__buffer + s->__bufsize;
  s->__get_limit = s->__buffer;

  /* Now use buffer to print.  */
  result = vfprintf (s, format, args);

  if (fflush (s) == EOF)
    result = -1;
  s->__buffer = s->__bufp = s->__get_limit = s->__put_limit = NULL;
  s->__bufsize = 0;

  return result;
}


/* Pads string with given number of a specified character.
   This code is taken from iopadn.c of the GNU I/O library.  */
#define PADSIZE 16
static const char blanks[PADSIZE] =
{' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
static const char zeroes[PADSIZE] =
{'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

ssize_t
__printf_pad (s, pad, count)
     FILE *s;
     char pad;
     int count;
{
  CONST char *padptr;
  register int i;
  size_t written = 0, w;

  padptr = pad == ' ' ? blanks : zeroes;

  for (i = count; i >= PADSIZE; i -= PADSIZE)
    {
      w = PUT(s, padptr, PADSIZE);
      written += w;
      if (w != PADSIZE)
	return written;
    }
  if (i > 0)
    {
      w = PUT(s, padptr, i);
      written += w;
    }
  return written;
}
#undef PADSIZE
#endif /* USE_IN_LIBIO */
