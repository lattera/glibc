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
along with this library; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

As a special exception, if you link this library with files
compiled with a GNU compiler to produce an executable, this does not cause
the resulting executable to be covered by the GNU General Public License.
This exception does not however invalidate any other reasons why
the executable file might be covered by the GNU General Public License. */

#include "libioP.h"
#include "libio.h"

int
_IO_vdprintf (d, format, arg)
     int d;
     const char *format;
     _IO_va_list arg;
{
  struct _IO_FILE_plus tmpfil;
  int done;

  _IO_init (&tmpfil.file, 0);
  _IO_JUMPS (&tmpfil.file) = &_IO_file_jumps;
  _IO_file_init (&tmpfil.file);
#if  !_IO_UNIFIED_JUMPTABLES
  tmpfil.vtable = NULL;
#endif
  if (_IO_file_attach (&tmpfil.file, d) == NULL)
    {
      _IO_un_link (&tmpfil.file);
      return EOF;
    }
  tmpfil.file._flags &= ~_IO_DELETE_DONT_CLOSE;

  tmpfil.file._IO_file_flags =
    _IO_mask_flags (&tmpfil.file, _IO_NO_READS,
		    _IO_NO_READS+_IO_NO_WRITES+_IO_IS_APPENDING);

  done = _IO_vfprintf (&tmpfil.file, format, arg);

  _IO_FINISH (&tmpfil.file);

  return done;
}
weak_alias (_IO_vdprintf, vdprintf)
