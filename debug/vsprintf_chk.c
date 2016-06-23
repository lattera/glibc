/* Copyright (C) 1994-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdarg.h>
#include <stdio.h>
#include "../libio/libioP.h"
#include "../libio/strfile.h"


static int _IO_str_chk_overflow (_IO_FILE *fp, int c) __THROW;

static int
_IO_str_chk_overflow (_IO_FILE *fp, int c)
{
  /* When we come to here this means the user supplied buffer is
     filled.  */
  __chk_fail ();
}


static const struct _IO_jump_t _IO_str_chk_jumps libio_vtable =
{
  JUMP_INIT_DUMMY,
  JUMP_INIT(finish, _IO_str_finish),
  JUMP_INIT(overflow, _IO_str_chk_overflow),
  JUMP_INIT(underflow, _IO_str_underflow),
  JUMP_INIT(uflow, _IO_default_uflow),
  JUMP_INIT(pbackfail, _IO_str_pbackfail),
  JUMP_INIT(xsputn, _IO_default_xsputn),
  JUMP_INIT(xsgetn, _IO_default_xsgetn),
  JUMP_INIT(seekoff, _IO_str_seekoff),
  JUMP_INIT(seekpos, _IO_default_seekpos),
  JUMP_INIT(setbuf, _IO_default_setbuf),
  JUMP_INIT(sync, _IO_default_sync),
  JUMP_INIT(doallocate, _IO_default_doallocate),
  JUMP_INIT(read, _IO_default_read),
  JUMP_INIT(write, _IO_default_write),
  JUMP_INIT(seek, _IO_default_seek),
  JUMP_INIT(close, _IO_default_close),
  JUMP_INIT(stat, _IO_default_stat),
  JUMP_INIT(showmanyc, _IO_default_showmanyc),
  JUMP_INIT(imbue, _IO_default_imbue)
};


int
___vsprintf_chk (char *s, int flags, size_t slen, const char *format,
		 va_list args)
{
  _IO_strfile f;
  int ret;
#ifdef _IO_MTSAFE_IO
  f._sbf._f._lock = NULL;
#endif

  if (slen == 0)
    __chk_fail ();

  _IO_no_init (&f._sbf._f, _IO_USER_LOCK, -1, NULL, NULL);
  _IO_JUMPS (&f._sbf) = &_IO_str_chk_jumps;
  s[0] = '\0';
  _IO_str_init_static_internal (&f, s, slen - 1, s);

  /* For flags > 0 (i.e. __USE_FORTIFY_LEVEL > 1) request that %n
     can only come from read-only format strings.  */
  if (flags > 0)
    f._sbf._f._flags2 |= _IO_FLAGS2_FORTIFY;

  ret = _IO_vfprintf (&f._sbf._f, format, args);

  *f._sbf._f._IO_write_ptr = '\0';
  return ret;
}
ldbl_hidden_def (___vsprintf_chk, __vsprintf_chk)
ldbl_strong_alias (___vsprintf_chk, __vsprintf_chk)
