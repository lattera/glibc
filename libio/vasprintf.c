/*
Copyright (C) 1995 Free Software Foundation

This file is part of the GNU IO Library.  This library is free
software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option)
any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

As a special exception, if you link this library with files
compiled with a GNU compiler to produce an executable, this does not cause
the resulting executable to be covered by the GNU General Public License.
This exception does not however invalidate any other reasons why
the executable file might be covered by the GNU General Public License. */

#include <malloc.h>
#include "libioP.h"
#include "stdio.h"
#include "strfile.h"

int
_IO_vasprintf (result_ptr, format, args)
     char **result_ptr;
     const char *format;
     _IO_va_list args;
{
  /* Initial size of the buffer to be used.  Will be doubled each time an
     overflow occurs.  */
  const _IO_size_t init_string_size = 100;
  char *string;
  _IO_strfile sf;
  int ret;
  string = ALLOC_BUF(init_string_size);
  if (string == NULL)
    return -1;
  _IO_init((_IO_FILE*)&sf, 0);
  _IO_JUMPS((_IO_FILE*)&sf) = &_IO_str_jumps;
  _IO_str_init_static ((_IO_FILE*)&sf, string, init_string_size, string);
  sf._f._flags &= ~_IO_USER_BUF;
  sf._s._allocate_buffer = (_IO_alloc_type)malloc;
  sf._s._free_buffer = (_IO_free_type)free;
  ret = _IO_vfprintf((_IO_FILE*)&sf, format, args);
  if (ret < 0)
    return ret;
  *result_ptr = (char*)realloc(sf._f._IO_buf_base,
			       (sf._f._IO_write_ptr - sf._f._IO_write_base) +1);
  if (*result_ptr == NULL)
    *result_ptr = sf._f._IO_buf_base;
  (*result_ptr)[sf._f._IO_write_ptr-sf._f._IO_write_base] = '\0';
  return ret;
}
weak_alias (_IO_vasprintf, vasprintf)
