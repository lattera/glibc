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

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <printf.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <printf.h>
#include <stddef.h>
#include "_itoa.h"
#include "../locale/localeinfo.h"

/* Include the shared code for parsing the format string.  */
#include "printf-parse.h"


/* This function from the GNU C library is also used in libio.
   To compile for use in libio, compile with -DUSE_IN_LIBIO.  */

#ifdef USE_IN_LIBIO
/* This code is for use in libio.  */
#include <libioP.h>
#define PUT(f, s, n)	_IO_sputn (f, s, n)
#define PAD(padchar)							      \
  if (specs[cnt].info.width > 0)					      \
    done += _IO_padn (s, padchar, specs[cnt].info.width)
#define PUTC(c, f)	_IO_putc (c, f)
#define vfprintf	_IO_vfprintf
#define size_t		_IO_size_t
#define FILE		_IO_FILE
#define va_list		_IO_va_list
#undef	BUFSIZ
#define BUFSIZ		_IO_BUFSIZ
#define ARGCHECK(s, format)						      \
  do									      \
    {									      \
      /* Check file argument for consistence.  */			      \
      CHECK_FILE (s, -1);						      \
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
ssize_t __printf_pad __P ((FILE *, char pad, size_t n));
#define PAD(padchar)							      \
  if (specs[cnt].info.width > 0)					      \
    { if (__printf_pad (s, padchar, specs[cnt].info.width) == -1)	      \
	return -1; else done += specs[cnt].info.width; }
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
      register const int outc = (x);					      \
      if (putc (outc, s) == EOF)					      \
	return -1;							      \
      else								      \
	++done;								      \
    } while (0)

#define outstring(string, len)						      \
  do									      \
    {									      \
      if (len > 20)							      \
	{								      \
	  if (PUT (s, string, len) != len)				      \
	    return -1;							      \
	  done += len;							      \
	}								      \
      else								      \
	{								      \
	  register const char *cp = string;				      \
	  register int l = len;						      \
	  while (l-- > 0)						      \
	    outchar (*cp++);						      \
	}								      \
    } while (0)

/* Helper function to provide temporary buffering for unbuffered streams.  */
static int buffered_vfprintf __P ((FILE *stream, const char *fmt, va_list));

static printf_function printf_unknown;

extern printf_function **__printf_function_table;

static char *group_number __P ((char *, char *, const char *, wchar_t));


int
vfprintf (s, format, ap)
    register FILE *s;
    const char *format;
    va_list ap;
{
  /* The character used as thousands separator.  */
  wchar_t thousands_sep;

  /* The string describing the size of groups of digits.  */
  const char *grouping;

  /* Array with information about the needed arguments.  This has to be
     dynamically extendable.  */
  size_t nspecs;
  size_t nspecs_max;
  struct printf_spec *specs;

  /* The number of arguments the format string requests.  This will
     determine the size of the array needed to store the argument
     attributes.  */
  size_t nargs;
  int *args_type;
  union printf_arg *args_value;

  /* Positional parameters refer to arguments directly.  This could also
     determine the maximum number of arguments.  Track the maximum number.  */
  size_t max_ref_arg;

  /* End of leading constant string.  */
  const char *lead_str_end;

  /* Number of characters written.  */
  register size_t done = 0;

  /* Running pointer through format string.  */
  const char *f;

  /* Just a counter.  */
  int cnt;

  ARGCHECK (s, format);

  if (UNBUFFERED_P (s))
    /* Use a helper function which will allocate a local temporary buffer
       for the stream and then call us again.  */
    return buffered_vfprintf (s, format, ap);

  /* Reset multibyte characters to their initial state.  */
  (void) mblen ((char *) NULL, 0);

  /* Figure out the thousands separator character.  */
  if (mbtowc (&thousands_sep, _NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP),
              strlen (_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP))) <= 0)
    thousands_sep = (wchar_t) *_NL_CURRENT (LC_NUMERIC, THOUSANDS_SEP);
  grouping = _NL_CURRENT (LC_NUMERIC, GROUPING);
  if (*grouping == '\0' || *grouping == CHAR_MAX || thousands_sep == L'\0')
    grouping = NULL;

  nspecs_max = 32;		/* A more or less arbitrary start value.  */
  specs = alloca (nspecs_max * sizeof (struct printf_spec));
  nspecs = 0;
  nargs = 0;
  max_ref_arg = 0;

  /* Find the first format specifier.  */
  lead_str_end = find_spec (format);

  for (f = lead_str_end; *f != '\0'; f = specs[nspecs++].next_fmt)
    {
      if (nspecs >= nspecs_max)
	{
	  /* Extend the array of format specifiers.  */
	  struct printf_spec *old = specs;

	  nspecs_max *= 2;
	  specs = alloca (nspecs_max * sizeof (struct printf_spec));
	  if (specs == &old[nspecs])
	    /* Stack grows up, OLD was the last thing allocated; extend it.  */
	    nspecs_max += nspecs_max / 2;
	  else
	    {
	      /* Copy the old array's elements to the new space.  */
	      memcpy (specs, old, nspecs * sizeof (struct printf_spec));
	      if (old == &specs[nspecs])
		/* Stack grows down, OLD was just below the new SPECS.
		   We can use that space when the new space runs out.  */
		nspecs_max += nspecs_max / 2;
	    }
	}

      /* Parse the format specifier.  */
      nargs += parse_one_spec (f, nargs, &specs[nspecs], &max_ref_arg);
    }

  /* Determine the number of arguments the format string consumes.  */
  nargs = MAX (nargs, max_ref_arg);

  /* Allocate memory for the argument descriptions.  */
  args_type = alloca (nargs * sizeof (int));
  args_value = alloca (nargs * sizeof (union printf_arg));

  /* XXX Could do sanity check here:
     Initialize args_type elts to zero.
     If any is still zero after this loop, format is invalid.  */

  /* Fill in the types of all the arguments.  */
  for (cnt = 0; cnt < nspecs; ++cnt)
    {
      /* If the width is determined by an argument this is an int.  */ 
      if (specs[cnt].width_arg != -1)
	args_type[specs[cnt].width_arg] = PA_INT;

      /* If the precision is determined by an argument this is an int.  */ 
      if (specs[cnt].prec_arg != -1)
	args_type[specs[cnt].prec_arg] = PA_INT;

      switch (specs[cnt].ndata_args)
	{
	case 0:			/* No arguments.  */
	  break;
	case 1:			/* One argument; we already have the type.  */
	  args_type[specs[cnt].data_arg] = specs[cnt].data_arg_type;
	  break;
	default:
	  /* We have more than one argument for this format spec.  We must
	     call the arginfo function again to determine all the types.  */
	  (void) (*__printf_arginfo_table[specs[cnt].info.spec])
	    (&specs[cnt].info,
	     specs[cnt].ndata_args, &args_type[specs[cnt].data_arg]);
	  break;
	}
    }

  /* Now we know all the types and the order.  Fill in the argument values.  */
  for (cnt = 0; cnt < nargs; ++cnt)
    switch (args_type[cnt])
      {
#define T(tag, mem, type)						      \
      case tag:								      \
	args_value[cnt].mem = va_arg (ap, type);			      \
	break

	T (PA_CHAR, pa_char, int); /* Promoted.  */
	T (PA_INT|PA_FLAG_SHORT, pa_short_int, int); /* Promoted.  */
	T (PA_INT, pa_int, int);
	T (PA_INT|PA_FLAG_LONG, pa_long_int, long int);
	T (PA_INT|PA_FLAG_LONG_LONG, pa_long_long_int, long long int);
	T (PA_FLOAT, pa_float, double);	/* Promoted.  */
	T (PA_DOUBLE, pa_double, double);
	T (PA_DOUBLE|PA_FLAG_LONG_DOUBLE, pa_long_double, long double);
	T (PA_STRING, pa_string, const char *);
	T (PA_POINTER, pa_pointer, void *);
#undef T
      default:
	if ((args_type[cnt] & PA_FLAG_PTR) != 0)
	  args_value[cnt].pa_pointer = va_arg (ap, void *);
	break;
      }

  /* Write the literal text before the first format.  */
  outstring (format, lead_str_end - format);

  /* Now walk through all format specifiers and process them.  */
  for (cnt = 0; cnt < nspecs; ++cnt)
    {
      printf_function *function; /* Auxiliary function to do output.  */
      int is_neg;		/* Decimal integer is negative.  */
      int base;			/* Base of a number to be written.  */
      unsigned long long int num; /* Integral number to be written.  */
      const char *str;		/* String to be written.  */
      char errorbuf[1024];      /* Buffer sometimes used by %m.  */

      if (specs[cnt].width_arg != -1)
	{
	  /* Extract the field width from an argument.  */
	  specs[cnt].info.width = args_value[specs[cnt].width_arg].pa_int;

	  if (specs[cnt].info.width < 0)
	    /* If the width value is negative left justification is selected
	       and the value is taken as being positive.  */
	    {
	      specs[cnt].info.width = -specs[cnt].info.width;
	      specs[cnt].info.left = 1;
	    }
	}

      if (specs[cnt].prec_arg != -1)
	{
	  /* Extract the precision from an argument.  */
	  specs[cnt].info.prec = args_value[specs[cnt].prec_arg].pa_int;

	  if (specs[cnt].info.prec < 0)
	    /* If the precision is negative the precision is omitted.  */
	    specs[cnt].info.prec = -1;
	}

      /* Check for a user-defined handler for this spec.  */
      function = (__printf_function_table == NULL ? NULL :
                  __printf_function_table[specs[cnt].info.spec]);

      if (function != NULL)
      use_function:		/* Built-in formats with helpers use this.  */
	{
	  int function_done;
	  unsigned int i;
	  const void *ptr[specs[cnt].ndata_args];

	  /* Fill in an array of pointers to the argument values.  */
	  for (i = 0; i < specs[cnt].ndata_args; ++i)
	    ptr[i] = &args_value[specs[cnt].data_arg + i];

	  /* Call the function.  */
	  function_done = (*function) (s, &specs[cnt].info, ptr);

	  /* If an error occured don't do any further work.  */
	  if (function_done < 0)
	    return -1;

	  done += function_done;
	}
      else
	switch (specs[cnt].info.spec)
	  {
	  case '%':
	    /* Write a literal "%".  */
	    outchar ('%');
	    break;
	  case 'i':
	  case 'd':
	    {
	      long long int signed_num;

	      /* Decimal integer.  */
	      base = 10;
	      if (specs[cnt].info.is_longlong)
		signed_num = args_value[specs[cnt].data_arg].pa_long_long_int;
	      else if (specs[cnt].info.is_long)
		signed_num = args_value[specs[cnt].data_arg].pa_long_int;
	      else if (!specs[cnt].info.is_short)
		signed_num = args_value[specs[cnt].data_arg].pa_int;
	      else
		signed_num = args_value[specs[cnt].data_arg].pa_short_int;

	      is_neg = signed_num < 0;
	      num = is_neg ? (- signed_num) : signed_num;
	      goto number;
	    }

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

            if (specs[cnt].info.is_longlong)
	      num = args_value[specs[cnt].data_arg].pa_u_long_long_int;
            else if (specs[cnt].info.is_long)
	      num = args_value[specs[cnt].data_arg].pa_u_long_int;
            else if (!specs[cnt].info.is_short)
	      num = args_value[specs[cnt].data_arg].pa_u_int;
            else
	      num = args_value[specs[cnt].data_arg].pa_u_short_int;

            /* ANSI only specifies the `+' and
               ` ' flags for signed conversions.  */
            is_neg = 0;
	    specs[cnt].info.showsign = 0;
	    specs[cnt].info.space = 0;

	  number:
	    /* Number of base BASE.  */
            {
              char work[BUFSIZ];
              char *const workend = &work[sizeof(work) - 1];
              register char *w;

              /* Supply a default precision if none was given.  */
              if (specs[cnt].info.prec == -1)
                specs[cnt].info.prec = 1;

              /* Put the number in WORK.  */
              w = _itoa (num, workend + 1, base, specs[cnt].info.spec == 'X');
	      w -= 1;
              if (specs[cnt].info.group && grouping)
                w = group_number (w, workend, grouping, thousands_sep);
              specs[cnt].info.width -= workend - w;
              specs[cnt].info.prec -= workend - w;

              if (num != 0 && specs[cnt].info.alt && base == 8
		  && specs[cnt].info.prec <= 0)
                {
		  /* Add octal marker.  */
                  *w-- = '0';
                  --specs[cnt].info.width;
                }

              if (specs[cnt].info.prec > 0)
                {
		  /* Add zeros to the precision.  */
                  specs[cnt].info.width -= specs[cnt].info.prec;
                  while (specs[cnt].info.prec-- > 0)
                    *w-- = '0';
                }

              if (num != 0 && specs[cnt].info.alt && base == 16)
		/* Account for 0X hex marker.  */
                specs[cnt].info.width -= 2;

              if (is_neg || specs[cnt].info.showsign || specs[cnt].info.space)
                --specs[cnt].info.width;

              if (!specs[cnt].info.left && specs[cnt].info.pad == ' ')
                PAD (' ');

              if (is_neg)
                outchar ('-');
              else if (specs[cnt].info.showsign)
                outchar ('+');
              else if (specs[cnt].info.space)
                outchar (' ');

              if (num != 0 && specs[cnt].info.alt && base == 16)
                {
                  outchar ('0');
                  outchar (specs[cnt].info.spec);
                }

              if (!specs[cnt].info.left && specs[cnt].info.pad == '0')
                PAD ('0');

              /* Write the number.  */
              while (++w <= workend)
                outchar (*w);

              if (specs[cnt].info.left)
                PAD (' ');
            }
            break;

          case 'e':
          case 'E':
          case 'f':
          case 'g':
          case 'G':
            {
              /* Floating-point number.  This is handled by printf_fp.c.  */
              extern printf_function __printf_fp;
              function = __printf_fp;
              goto use_function;
            }

          case 'c':
            /* Character.  */
	    --specs[cnt].info.width;/* Account for the character itself.  */
            if (!specs[cnt].info.left)
	      PAD (' ');
            outchar ((unsigned char) args_value[specs[cnt].data_arg].pa_char);
            if (specs[cnt].info.left)
              PAD (' ');
            break;

          case 's':
            {
              static const char null[] = "(null)";
              size_t len;

	      str = args_value[specs[cnt].data_arg].pa_string;

	    string:

              if (str == NULL)
		{
		  /* Write "(null)" if there's space.  */
		  if (specs[cnt].info.prec == -1
		      || specs[cnt].info.prec >= (int) sizeof (null) - 1)
		    {
		      str = null;
		      len = sizeof (null) - 1;
		    }
		  else
		    {
		      str = "";
		      len = 0;
		    }
		}
              else if (specs[cnt].info.prec != -1)
		{
		  /* Search for the end of the string, but don't search
		     past the length specified by the precision.  */
		  const char *end = memchr (str, '\0', specs[cnt].info.prec);
		  if (end)
		    len = end - str;
		  else
		    len = specs[cnt].info.prec;
		}
	      else
		len = strlen (str);

              specs[cnt].info.width -= len;

              if (!specs[cnt].info.left)
                PAD (' ');
              outstring (str, len);
              if (specs[cnt].info.left)
                PAD (' ');
            }
            break;

          case 'p':
            /* Generic pointer.  */
            {
              const void *ptr;
              ptr = args_value[specs[cnt].data_arg].pa_pointer;
              if (ptr != NULL)
                {
                  /* If the pointer is not NULL, write it as a %#x spec.  */
                  base = 16;
                  num = (unsigned long long int) (unsigned long int) ptr;
                  is_neg = 0;
                  specs[cnt].info.alt = 1;
		  specs[cnt].info.spec = 'x';
                  specs[cnt].info.group = 0;
                  goto number;
                }
              else
                {
                  /* Write "(nil)" for a nil pointer.  */
                  str = "(nil)";
		  /* Make sure the full string "(nil)" is printed.  */
		  if (specs[cnt].info.prec < 5)
		    specs[cnt].info.prec = 5;
                  goto string;
                }
            }
            break;

          case 'n':
            /* Answer the count of characters written.  */
            if (specs[cnt].info.is_longlong)
	      *(long long int *) 
		args_value[specs[cnt].data_arg].pa_pointer = done;
            else if (specs[cnt].info.is_long)
	      *(long int *) 
		args_value[specs[cnt].data_arg].pa_pointer = done;
            else if (!specs[cnt].info.is_short)
	      *(int *) 
		args_value[specs[cnt].data_arg].pa_pointer = done;
            else
	      *(short int *) 
		args_value[specs[cnt].data_arg].pa_pointer = done;
            break;

          case 'm':
            {
              extern char *_strerror_internal __P ((int, char *buf, size_t));
              str = _strerror_internal (errno, errorbuf, sizeof errorbuf);
              goto string;
            }

          default:
            /* Unrecognized format specifier.  */
            function = printf_unknown;
            goto use_function;
	  }

      /* Write the following constant string.  */
      outstring (specs[cnt].end_of_fmt,
		 specs[cnt].next_fmt - specs[cnt].end_of_fmt);
    }

  return done;
}


/* Handle an unknown format specifier.  This prints out a canonicalized
   representation of the format spec itself.  */

static int
printf_unknown (s, info, args)
  FILE *s;
  const struct printf_info *info;
  const void **const args;
{
  int done = 0;
  char work[BUFSIZ];
  char *const workend = &work[sizeof(work) - 1];
  register char *w;

  outchar ('%');

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

  if (info->width != 0)
    {
      w = _itoa (info->width, workend + 1, 10, 0);
      while (++w <= workend)
	outchar (*w);
    }

  if (info->prec != -1)
    {
      outchar ('.');
      w = _itoa (info->prec, workend + 1, 10, 0);
      while (++w <= workend)
	outchar (*w);
    }

  if (info->spec != '\0')
    outchar (info->spec);

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
_IO_helper_overflow (s, c)
  _IO_FILE *s;
  int c;
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
buffered_vfprintf (s, format, args)
  register _IO_FILE *s;
  char const *format;
  _IO_va_list args;
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
buffered_vfprintf (s, format, args)
  register FILE *s;
  char const *format;
  va_list args;
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
     size_t count;
{
  const char *padptr;
  register size_t i;

  padptr = pad == ' ' ? blanks : zeroes;

  for (i = count; i >= PADSIZE; i -= PADSIZE)
    if (PUT (s, padptr, PADSIZE) != PADSIZE)
      return -1;
  if (i > 0)
    if (PUT (s, padptr, i) != i)
      return -1;

  return count;
}
#undef PADSIZE
#endif /* USE_IN_LIBIO */
