/* Copyright (C) 1991, 1995, 1997, 1998, 2004, 2006, 2009
   Free Software Foundation, Inc.
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
#include <stdio.h>
#include "../libio/libioP.h"
#include "../libio/strfile.h"

extern const struct _IO_jump_t _IO_strn_jumps attribute_hidden;

/* Write formatted output into S, according to the format
   string FORMAT, writing no more than MAXLEN characters.  */
/* VARARGS5 */
int
___vsnprintf_chk (char *s, size_t maxlen, int flags, size_t slen,
		  const char *format, va_list args)
{
  /* XXX Maybe for less strict version do not fail immediately.
     Though, maxlen is supposed to be the size of buffer pointed
     to by s, so a conforming program can't pass such maxlen
     to *snprintf.  */
  if (__builtin_expect (slen < maxlen, 0))
    __chk_fail ();

  _IO_strnfile sf;
  int ret;
#ifdef _IO_MTSAFE_IO
  sf.f._sbf._f._lock = NULL;
#endif

  /* We need to handle the special case where MAXLEN is 0.  Use the
     overflow buffer right from the start.  */
  if (maxlen == 0)
    {
      s = sf.overflow_buf;
      maxlen = sizeof (sf.overflow_buf);
    }

  _IO_no_init (&sf.f._sbf._f, _IO_USER_LOCK, -1, NULL, NULL);
  _IO_JUMPS (&sf.f._sbf) = &_IO_strn_jumps;
  s[0] = '\0';

  /* For flags > 0 (i.e. __USE_FORTIFY_LEVEL > 1) request that %n
     can only come from read-only format strings.  */
  if (flags > 0)
    sf.f._sbf._f._flags2 |= _IO_FLAGS2_FORTIFY;

  _IO_str_init_static_internal (&sf.f, s, maxlen - 1, s);
  ret = INTUSE(_IO_vfprintf) (&sf.f._sbf._f, format, args);

  if (sf.f._sbf._f._IO_buf_base != sf.overflow_buf)
    *sf.f._sbf._f._IO_write_ptr = '\0';
  return ret;
}
ldbl_hidden_def (___vsnprintf_chk, __vsnprintf_chk)
ldbl_strong_alias (___vsnprintf_chk, __vsnprintf_chk)
