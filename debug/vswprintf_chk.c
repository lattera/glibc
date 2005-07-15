/* Copyright (C) 1991,1995,1997,1998,2004,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdarg.h>
#include <wchar.h>
#include "../libio/libioP.h"
#include "../libio/strfile.h"


/* Write formatted output into S, according to the format
   string FORMAT, writing no more than MAXLEN characters.  */
/* VARARGS5 */
int
__vswprintf_chk (wchar_t *s, size_t maxlen, int flags, size_t slen,
		 const wchar_t *format, va_list args)
{
  /* XXX Maybe for less strict version do not fail immediately.
     Though, maxlen is supposed to be the size of buffer pointed
     to by s, so a conforming program can't pass such maxlen
     to *snprintf.  */
  if (__builtin_expect (slen < maxlen, 0))
    __chk_fail ();

  _IO_wstrnfile sf;
  struct _IO_wide_data wd;
  int ret;
#ifdef _IO_MTSAFE_IO
  sf.f._sbf._f._lock = NULL;
#endif

  /* We need to handle the special case where MAXLEN is 0.  Use the
     overflow buffer right from the start.  */
  if (__builtin_expect (maxlen == 0, 0))
    /* Since we have to write at least the terminating L'\0' a buffer
       length of zero always makes the function fail.  */
    return -1;

  _IO_no_init (&sf.f._sbf._f, _IO_USER_LOCK, 0, &wd, &_IO_wstrn_jumps);
  _IO_fwide (&sf.f._sbf._f, 1);
  s[0] = L'\0';

  /* For flags > 0 (i.e. __USE_FORTIFY_LEVEL > 1) request that %n
     can only come from read-only format strings.  */
  if (flags > 0)
    sf.f._sbf._f._flags2 |= _IO_FLAGS2_FORTIFY;

  _IO_wstr_init_static (&sf.f._sbf._f, s, maxlen - 1, s);
  ret = _IO_vfwprintf ((_IO_FILE *) &sf.f._sbf, format, args);

  if (sf.f._sbf._f._wide_data->_IO_buf_base == sf.overflow_buf)
    /* ISO C99 requires swprintf/vswprintf to return an error if the
       output does not fit int he provided buffer.  */
    return -1;

  /* Terminate the string.  */
  *sf.f._sbf._f._wide_data->_IO_write_ptr = '\0';

  return ret;
}
libc_hidden_def (__vswprintf_chk)
