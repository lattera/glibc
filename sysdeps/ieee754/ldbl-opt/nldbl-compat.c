/* *printf* family compatibility routines for IEEE double as long double
   Copyright (C) 2006-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@cygnus.com>, 2006.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include <libioP.h>
#include <wchar.h>
#include <printf.h>
#include <monetary.h>
#include <locale/localeinfo.h>
#include <sys/syslog.h>
#include <libc-lock.h>

#include "nldbl-compat.h"

libc_hidden_proto (__nldbl_vfprintf)
libc_hidden_proto (__nldbl_vsscanf)
libc_hidden_proto (__nldbl_vsprintf)
libc_hidden_proto (__nldbl_vfscanf)
libc_hidden_proto (__nldbl_vfwscanf)
libc_hidden_proto (__nldbl_vdprintf)
libc_hidden_proto (__nldbl_vswscanf)
libc_hidden_proto (__nldbl_vfwprintf)
libc_hidden_proto (__nldbl_vswprintf)
libc_hidden_proto (__nldbl_vsnprintf)
libc_hidden_proto (__nldbl_vasprintf)
libc_hidden_proto (__nldbl_obstack_vprintf)
libc_hidden_proto (__nldbl___vfwprintf_chk)
libc_hidden_proto (__nldbl___vsnprintf_chk)
libc_hidden_proto (__nldbl___vfprintf_chk)
libc_hidden_proto (__nldbl___vsyslog_chk)
libc_hidden_proto (__nldbl___vsprintf_chk)
libc_hidden_proto (__nldbl___vswprintf_chk)
libc_hidden_proto (__nldbl___vasprintf_chk)
libc_hidden_proto (__nldbl___vdprintf_chk)
libc_hidden_proto (__nldbl___obstack_vprintf_chk)
libc_hidden_proto (__nldbl___vstrfmon)
libc_hidden_proto (__nldbl___vstrfmon_l)
libc_hidden_proto (__nldbl___isoc99_vsscanf)
libc_hidden_proto (__nldbl___isoc99_vfscanf)
libc_hidden_proto (__nldbl___isoc99_vswscanf)
libc_hidden_proto (__nldbl___isoc99_vfwscanf)

static void
__nldbl_cleanup (void *arg)
{
  __no_long_double = 0;
}

#define set_no_long_double() \
  __libc_cleanup_push (__nldbl_cleanup, NULL); __no_long_double = 1
#define clear_no_long_double() \
  __no_long_double = 0; __libc_cleanup_pop (0)

/* Compatibility with IEEE double as long double.
   IEEE quad long double is used by default for most programs, so
   we don't need to split this into one file per function for the
   sake of statically linked programs.  */

int
attribute_compat_text_section
__nldbl___asprintf (char **string_ptr, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vasprintf (string_ptr, fmt, arg);
  va_end (arg);

  return done;
}
weak_alias (__nldbl___asprintf, __nldbl_asprintf)

int
attribute_compat_text_section
__nldbl_dprintf (int d, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vdprintf (d, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_fprintf (FILE *stream, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfprintf (stream, fmt, arg);
  va_end (arg);

  return done;
}
weak_alias (__nldbl_fprintf, __nldbl__IO_fprintf)

int
attribute_compat_text_section weak_function
__nldbl_fwprintf (FILE *stream, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwprintf (stream, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_printf (const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfprintf (stdout, fmt, arg);
  va_end (arg);

  return done;
}
strong_alias (__nldbl_printf, __nldbl__IO_printf)

int
attribute_compat_text_section
__nldbl_sprintf (char *s, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vsprintf (s, fmt, arg);
  va_end (arg);

  return done;
}
strong_alias (__nldbl_sprintf, __nldbl__IO_sprintf)

int
attribute_compat_text_section
__nldbl_vfprintf (FILE *s, const char *fmt, va_list ap)
{
  int done;
  set_no_long_double ();
  done = _IO_vfprintf (s, fmt, ap);
  clear_no_long_double ();
  return done;
}
libc_hidden_def (__nldbl_vfprintf)
strong_alias (__nldbl_vfprintf, __nldbl__IO_vfprintf)

int
attribute_compat_text_section
__nldbl__IO_vsprintf (char *string, const char *fmt, va_list ap)
{
  int done;
  __no_long_double = 1;
  done = _IO_vsprintf (string, fmt, ap);
  __no_long_double = 0;
  return done;
}
weak_alias (__nldbl__IO_vsprintf, __nldbl_vsprintf)
libc_hidden_def (__nldbl_vsprintf)

int
attribute_compat_text_section
__nldbl_obstack_vprintf (struct obstack *obstack, const char *fmt,
			 va_list ap)
{
  int done;
  __no_long_double = 1;
  done = _IO_obstack_vprintf (obstack, fmt, ap);
  __no_long_double = 0;
  return done;
}
libc_hidden_def (__nldbl_obstack_vprintf)

int
attribute_compat_text_section
__nldbl_obstack_printf (struct obstack *obstack, const char *fmt, ...)
{
  int result;
  va_list ap;
  va_start (ap, fmt);
  result = __nldbl_obstack_vprintf (obstack, fmt, ap);
  va_end (ap);
  return result;
}

int
attribute_compat_text_section weak_function
__nldbl_snprintf (char *s, size_t maxlen, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vsnprintf (s, maxlen, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_swprintf (wchar_t *s, size_t n, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vswprintf (s, n, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section weak_function
__nldbl_vasprintf (char **result_ptr, const char *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = _IO_vasprintf (result_ptr, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl_vasprintf)

int
attribute_compat_text_section
__nldbl_vdprintf (int d, const char *fmt, va_list arg)
{
  int res;
  set_no_long_double ();
  res = _IO_vdprintf (d, fmt, arg);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl_vdprintf)

int
attribute_compat_text_section weak_function
__nldbl_vfwprintf (FILE *s, const wchar_t *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = _IO_vfwprintf (s, fmt, ap);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl_vfwprintf)

int
attribute_compat_text_section
__nldbl_vprintf (const char *fmt, va_list ap)
{
  return __nldbl_vfprintf (stdout, fmt, ap);
}

int
attribute_compat_text_section
__nldbl_vsnprintf (char *string, size_t maxlen, const char *fmt,
		   va_list ap)
{
  int res;
  __no_long_double = 1;
  res = _IO_vsnprintf (string, maxlen, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl_vsnprintf)
weak_alias (__nldbl_vsnprintf, __nldbl___vsnprintf)

int
attribute_compat_text_section weak_function
__nldbl_vswprintf (wchar_t *string, size_t maxlen, const wchar_t *fmt,
		   va_list ap)
{
  int res;
  __no_long_double = 1;
  res = _IO_vswprintf (string, maxlen, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl_vswprintf)

int
attribute_compat_text_section
__nldbl_vwprintf (const wchar_t *fmt, va_list ap)
{
  return __nldbl_vfwprintf (stdout, fmt, ap);
}

int
attribute_compat_text_section
__nldbl_wprintf (const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwprintf (stdout, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl__IO_vfscanf (FILE *s, const char *fmt, _IO_va_list ap,
		    int *errp)
{
  int res;
  set_no_long_double ();
  res = _IO_vfscanf (s, fmt, ap, errp);
  clear_no_long_double ();
  return res;
}

int
attribute_compat_text_section
__nldbl___vfscanf (FILE *s, const char *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = _IO_vfscanf (s, fmt, ap, NULL);
  clear_no_long_double ();
  return res;
}
weak_alias (__nldbl___vfscanf, __nldbl_vfscanf)
libc_hidden_def (__nldbl_vfscanf)

int
attribute_compat_text_section
__nldbl_sscanf (const char *s, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vsscanf (s, fmt, arg);
  va_end (arg);

  return done;
}
strong_alias (__nldbl_sscanf, __nldbl__IO_sscanf)

int
attribute_compat_text_section
__nldbl___vsscanf (const char *string, const char *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = _IO_vsscanf (string, fmt, ap);
  __no_long_double = 0;
  return res;
}
weak_alias (__nldbl___vsscanf, __nldbl_vsscanf)
libc_hidden_def (__nldbl_vsscanf)

int
attribute_compat_text_section weak_function
__nldbl_vscanf (const char *fmt, va_list ap)
{
  return __nldbl_vfscanf (stdin, fmt, ap);
}

int
attribute_compat_text_section
__nldbl_fscanf (FILE *stream, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfscanf (stream, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_scanf (const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_vfwscanf (FILE *s, const wchar_t *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = _IO_vfwscanf (s, fmt, ap, NULL);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl_vfwscanf)

int
attribute_compat_text_section
__nldbl_swscanf (const wchar_t *s, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vswscanf (s, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_vswscanf (const wchar_t *string, const wchar_t *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = vswscanf (string, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl_vswscanf)

int
attribute_compat_text_section weak_function
__nldbl_vwscanf (const wchar_t *fmt, va_list ap)
{
  return __nldbl_vfwscanf (stdin, fmt, ap);
}

int
attribute_compat_text_section
__nldbl_fwscanf (FILE *stream, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwscanf (stream, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl_wscanf (const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___fprintf_chk (FILE *stream, int flag, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vfprintf_chk (stream, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___fwprintf_chk (FILE *stream, int flag, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vfwprintf_chk (stream, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___printf_chk (int flag, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vfprintf_chk (stdout, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___snprintf_chk (char *s, size_t maxlen, int flag, size_t slen,
			const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vsnprintf_chk (s, maxlen, flag, slen, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___sprintf_chk (char *s, int flag, size_t slen, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vsprintf_chk (s, flag, slen, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___swprintf_chk (wchar_t *s, size_t n, int flag, size_t slen,
			const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vswprintf_chk (s, n, flag, slen, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___vfprintf_chk (FILE *s, int flag, const char *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = __vfprintf_chk (s, flag, fmt, ap);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl___vfprintf_chk)

int
attribute_compat_text_section
__nldbl___vfwprintf_chk (FILE *s, int flag, const wchar_t *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = __vfwprintf_chk (s, flag, fmt, ap);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl___vfwprintf_chk)

int
attribute_compat_text_section
__nldbl___vprintf_chk (int flag, const char *fmt, va_list ap)
{
  return __nldbl___vfprintf_chk (stdout, flag, fmt, ap);
}

int
attribute_compat_text_section
__nldbl___vsnprintf_chk (char *string, size_t maxlen, int flag, size_t slen,
			 const char *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = __vsnprintf_chk (string, maxlen, flag, slen, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___vsnprintf_chk)

int
attribute_compat_text_section
__nldbl___vsprintf_chk (char *string, int flag, size_t slen, const char *fmt,
			va_list ap)
{
  int res;
  __no_long_double = 1;
  res = __vsprintf_chk (string, flag, slen, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___vsprintf_chk)

int
attribute_compat_text_section
__nldbl___vswprintf_chk (wchar_t *string, size_t maxlen, int flag, size_t slen,
			 const wchar_t *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = __vswprintf_chk (string, maxlen, flag, slen, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___vswprintf_chk)

int
attribute_compat_text_section
__nldbl___vwprintf_chk (int flag, const wchar_t *fmt, va_list ap)
{
  return __nldbl___vfwprintf_chk (stdout, flag, fmt, ap);
}

int
attribute_compat_text_section
__nldbl___wprintf_chk (int flag, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vfwprintf_chk (stdout, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___vasprintf_chk (char **ptr, int flag, const char *fmt, va_list arg)
{
  int res;
  __no_long_double = 1;
  res = __vasprintf_chk (ptr, flag, fmt, arg);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___vasprintf_chk)

int
attribute_compat_text_section
__nldbl___asprintf_chk (char **ptr, int flag, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vasprintf_chk (ptr, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___vdprintf_chk (int d, int flag, const char *fmt, va_list arg)
{
  int res;
  set_no_long_double ();
  res = __vdprintf_chk (d, flag, fmt, arg);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl___vdprintf_chk)

int
attribute_compat_text_section
__nldbl___dprintf_chk (int d, int flag, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___vdprintf_chk (d, flag, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___obstack_vprintf_chk (struct obstack *obstack, int flag,
			       const char *fmt, va_list arg)
{
  int res;
  __no_long_double = 1;
  res = __obstack_vprintf_chk (obstack, flag, fmt, arg);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___obstack_vprintf_chk)

int
attribute_compat_text_section
__nldbl___obstack_printf_chk (struct obstack *obstack, int flag,
			      const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___obstack_vprintf_chk (obstack, flag, fmt, arg);
  va_end (arg);

  return done;
}

extern __typeof (printf_size) __printf_size;

int
attribute_compat_text_section
__nldbl_printf_size (FILE *fp, const struct printf_info *info,
		     const void *const *args)
{
  struct printf_info info_no_ldbl = *info;

  info_no_ldbl.is_long_double = 0;
  return __printf_size (fp, &info_no_ldbl, args);
}

extern __typeof (__printf_fp) ___printf_fp;

int
attribute_compat_text_section
__nldbl___printf_fp (FILE *fp, const struct printf_info *info,
		     const void *const *args)
{
  struct printf_info info_no_ldbl = *info;

  info_no_ldbl.is_long_double = 0;
  return ___printf_fp (fp, &info_no_ldbl, args);
}

ssize_t
attribute_compat_text_section
__nldbl_strfmon (char *s, size_t maxsize, const char *format, ...)
{
  va_list ap;
  ssize_t res;

  va_start (ap, format);
  res = __nldbl___vstrfmon (s, maxsize, format, ap);
  va_end (ap);
  return res;
}

ssize_t
attribute_compat_text_section
__nldbl___strfmon_l (char *s, size_t maxsize, __locale_t loc,
		     const char *format, ...)
{
  va_list ap;
  ssize_t res;

  va_start (ap, format);
  res = __nldbl___vstrfmon_l (s, maxsize, loc, format, ap);
  va_end (ap);
  return res;
}
weak_alias (__nldbl___strfmon_l, __nldbl_strfmon_l)

ssize_t
attribute_compat_text_section
__nldbl___vstrfmon (char *s, size_t maxsize, const char *format, va_list ap)
{
  ssize_t res;
  __no_long_double = 1;
  res = __vstrfmon_l (s, maxsize, _NL_CURRENT_LOCALE, format, ap);
  __no_long_double = 0;
  va_end (ap);
  return res;
}
libc_hidden_def (__nldbl___vstrfmon)

ssize_t
attribute_compat_text_section
__nldbl___vstrfmon_l (char *s, size_t maxsize, __locale_t loc,
		      const char *format, va_list ap)
{
  ssize_t res;
  __no_long_double = 1;
  res = __vstrfmon_l (s, maxsize, loc, format, ap);
  __no_long_double = 0;
  va_end (ap);
  return res;
}
libc_hidden_def (__nldbl___vstrfmon_l)

void
attribute_compat_text_section
__nldbl_syslog (int pri, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  __nldbl___vsyslog_chk (pri, -1, fmt, ap);
  va_end (ap);
}

void
attribute_compat_text_section
__nldbl___syslog_chk (int pri, int flag, const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  __nldbl___vsyslog_chk (pri, flag, fmt, ap);
  va_end(ap);
}

void
attribute_compat_text_section
__nldbl___vsyslog_chk (int pri, int flag, const char *fmt, va_list ap)
{
  set_no_long_double ();
  __vsyslog_chk (pri, flag, fmt, ap);
  clear_no_long_double ();
}
libc_hidden_def (__nldbl___vsyslog_chk)

void
attribute_compat_text_section
__nldbl_vsyslog (int pri, const char *fmt, va_list ap)
{
  __nldbl___vsyslog_chk (pri, -1, fmt, ap);
}

int
attribute_compat_text_section
__nldbl___isoc99_vfscanf (FILE *s, const char *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = __isoc99_vfscanf (s, fmt, ap);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl___isoc99_vfscanf)

int
attribute_compat_text_section
__nldbl___isoc99_sscanf (const char *s, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vsscanf (s, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___isoc99_vsscanf (const char *string, const char *fmt, va_list ap)
{
  int res;
  __no_long_double = 1;
  res = __isoc99_vsscanf (string, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___isoc99_vsscanf)

int
attribute_compat_text_section
__nldbl___isoc99_vscanf (const char *fmt, va_list ap)
{
  return __nldbl___isoc99_vfscanf (stdin, fmt, ap);
}

int
attribute_compat_text_section
__nldbl___isoc99_fscanf (FILE *stream, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vfscanf (stream, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___isoc99_scanf (const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vfscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___isoc99_vfwscanf (FILE *s, const wchar_t *fmt, va_list ap)
{
  int res;
  set_no_long_double ();
  res = __isoc99_vfwscanf (s, fmt, ap);
  clear_no_long_double ();
  return res;
}
libc_hidden_def (__nldbl___isoc99_vfwscanf)

int
attribute_compat_text_section
__nldbl___isoc99_swscanf (const wchar_t *s, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vswscanf (s, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___isoc99_vswscanf (const wchar_t *string, const wchar_t *fmt,
			   va_list ap)
{
  int res;
  __no_long_double = 1;
  res = __isoc99_vswscanf (string, fmt, ap);
  __no_long_double = 0;
  return res;
}
libc_hidden_def (__nldbl___isoc99_vswscanf)

int
attribute_compat_text_section
__nldbl___isoc99_vwscanf (const wchar_t *fmt, va_list ap)
{
  return __nldbl___isoc99_vfwscanf (stdin, fmt, ap);
}

int
attribute_compat_text_section
__nldbl___isoc99_fwscanf (FILE *stream, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vfwscanf (stream, fmt, arg);
  va_end (arg);

  return done;
}

int
attribute_compat_text_section
__nldbl___isoc99_wscanf (const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl___isoc99_vfwscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}

#if LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __nldbl__IO_printf, _IO_printf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_sprintf, _IO_sprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_vfprintf, _IO_vfprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_vsprintf, _IO_vsprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_dprintf, dprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_fprintf, fprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_printf, printf, GLIBC_2_0);
compat_symbol (libc, __nldbl_sprintf, sprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vfprintf, vfprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vprintf, vprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_fprintf, _IO_fprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl___vsnprintf, __vsnprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_asprintf, asprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_obstack_printf, obstack_printf, GLIBC_2_0);
compat_symbol (libc, __nldbl_obstack_vprintf, obstack_vprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_snprintf, snprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vasprintf, vasprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vdprintf, vdprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vsnprintf, vsnprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vsprintf, vsprintf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_sscanf, _IO_sscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl__IO_vfscanf, _IO_vfscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl___vfscanf, __vfscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl___vsscanf, __vsscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_fscanf, fscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_scanf, scanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_sscanf, sscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vfscanf, vfscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vscanf, vscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl_vsscanf, vsscanf, GLIBC_2_0);
compat_symbol (libc, __nldbl___printf_fp, __printf_fp, GLIBC_2_0);
compat_symbol (libc, __nldbl_strfmon, strfmon, GLIBC_2_0);
compat_symbol (libc, __nldbl_syslog, syslog, GLIBC_2_0);
compat_symbol (libc, __nldbl_vsyslog, vsyslog, GLIBC_2_0);
#endif
#if LONG_DOUBLE_COMPAT(libc, GLIBC_2_1)
compat_symbol (libc, __nldbl___asprintf, __asprintf, GLIBC_2_1);
compat_symbol (libc, __nldbl_printf_size, printf_size, GLIBC_2_1);
compat_symbol (libc, __nldbl___strfmon_l, __strfmon_l, GLIBC_2_1);
#endif
#if LONG_DOUBLE_COMPAT(libc, GLIBC_2_2)
compat_symbol (libc, __nldbl_swprintf, swprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vwprintf, vwprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_wprintf, wprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_fwprintf, fwprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vfwprintf, vfwprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vswprintf, vswprintf, GLIBC_2_2);
compat_symbol (libc, __nldbl_fwscanf, fwscanf, GLIBC_2_2);
compat_symbol (libc, __nldbl_swscanf, swscanf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vfwscanf, vfwscanf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vswscanf, vswscanf, GLIBC_2_2);
compat_symbol (libc, __nldbl_vwscanf, vwscanf, GLIBC_2_2);
compat_symbol (libc, __nldbl_wscanf, wscanf, GLIBC_2_2);
#endif
#if LONG_DOUBLE_COMPAT(libc, GLIBC_2_3)
compat_symbol (libc, __nldbl_strfmon_l, strfmon_l, GLIBC_2_3);
#endif
#if LONG_DOUBLE_COMPAT(libc, GLIBC_2_3_4)
compat_symbol (libc, __nldbl___sprintf_chk, __sprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___vsprintf_chk, __vsprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___snprintf_chk, __snprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___vsnprintf_chk, __vsnprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___printf_chk, __printf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___fprintf_chk, __fprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___vprintf_chk, __vprintf_chk, GLIBC_2_3_4);
compat_symbol (libc, __nldbl___vfprintf_chk, __vfprintf_chk, GLIBC_2_3_4);
#endif
