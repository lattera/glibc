/* Copyright (C) 1995, 1997-2000, 2001 Free Software Foundation, Inc.
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
   02111-1307 USA.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include "libioP.h"
#include "libio.h"
#include <stdio_ext.h>

int
_IO_vdprintf (d, format, arg)
     int d;
     const char *format;
     _IO_va_list arg;
{
  struct _IO_FILE_plus tmpfil;
#ifdef _IO_MTSAFE_IO
  _IO_lock_t lock;
#endif
  struct _IO_wide_data wd;
  int done;

#ifdef _IO_MTSAFE_IO
  tmpfil.file._lock = &lock;
#endif
  _IO_no_init (&tmpfil.file, 0, 0, &wd, &_IO_wfile_jumps);
  _IO_JUMPS (&tmpfil) = &_IO_file_jumps;
  _IO_file_init (&tmpfil);
#if  !_IO_UNIFIED_JUMPTABLES
  tmpfil.vtable = NULL;
#endif
  if (_IO_file_attach ((_IO_FILE *) &tmpfil, d) == NULL)
    {
      _IO_un_link (&tmpfil);
      return EOF;
    }
  tmpfil.file._IO_file_flags =
    (_IO_mask_flags (&tmpfil.file, _IO_NO_READS,
		     _IO_NO_READS+_IO_NO_WRITES+_IO_IS_APPENDING)
     | _IO_DELETE_DONT_CLOSE);
#ifdef _IO_MTSAFE_IO
  __fsetlocking ((FILE *) &tmpfil, FSETLOCKING_BYCALLER);
#endif

  done = _IO_vfprintf ((_IO_FILE *) &tmpfil, format, arg);

  _IO_FINISH ((_IO_FILE *) &tmpfil);

  return done;
}

#ifdef weak_alias
weak_alias (_IO_vdprintf, vdprintf)
#endif
