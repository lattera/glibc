/*
Copyright (C) 1993, 1995 Free Software Foundation

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

#include <libioP.h>
#include <stdio.h>
#include <stdlib.h>


/* Prototyped for local functions.  */
static _IO_ssize_t _IO_cookie_read __P ((register _IO_FILE* fp, void* buf,
					 _IO_ssize_t size));
static _IO_ssize_t _IO_cookie_write __P ((register _IO_FILE* fp,
					  const void* buf, _IO_ssize_t size));
static _IO_fpos_t _IO_cookie_seek __P ((_IO_FILE *fp, _IO_off_t offset,
					int dir));
static int _IO_cookie_close __P ((_IO_FILE* fp));


static _IO_ssize_t
_IO_cookie_read (fp, buf, size)
     register _IO_FILE* fp;
     void* buf;
     _IO_ssize_t size;
{
  struct _IO_cookie_file *cfile = (struct _IO_cookie_file *) fp;

  if (cfile->io_functions.read == NULL)
    return -1;

  return cfile->io_functions.read (cfile->cookie, buf, size);
}

static _IO_ssize_t
_IO_cookie_write (fp, buf, size)
     register _IO_FILE* fp;
     const void* buf;
     _IO_ssize_t size;
{
  struct _IO_cookie_file *cfile = (struct _IO_cookie_file *) fp;

  if (cfile->io_functions.write == NULL)
    return -1;

  return cfile->io_functions.write (cfile->cookie, buf, size);
}

static _IO_fpos_t
_IO_cookie_seek (fp, offset, dir)
     _IO_FILE *fp;
     _IO_off_t offset;
     int dir;
{
  struct _IO_cookie_file *cfile = (struct _IO_cookie_file *) fp;
  _IO_fpos_t pos;

  if (cfile->io_functions.seek == NULL)
    return _IO_pos_BAD;

  pos = _IO_pos_0;
  _IO_pos_adjust (pos, offset);

  if (cfile->io_functions.seek (cfile->cookie, pos, dir))
    return _IO_pos_BAD;

  return pos;
}

static int
_IO_cookie_close (fp)
     _IO_FILE* fp;
{
  struct _IO_cookie_file *cfile = (struct _IO_cookie_file *) fp;

  if (cfile->io_functions.close == NULL)
    return 0;

  return cfile->io_functions.close (cfile->cookie);
}


static struct _IO_jump_t _IO_cookie_jumps = {
  JUMP_INIT_DUMMY,
  JUMP_INIT(finish, _IO_file_finish),
  JUMP_INIT(overflow, _IO_file_overflow),
  JUMP_INIT(underflow, _IO_file_underflow),
  JUMP_INIT(uflow, _IO_default_uflow),
  JUMP_INIT(pbackfail, _IO_default_pbackfail),
  JUMP_INIT(xsputn, _IO_file_xsputn),
  JUMP_INIT(xsgetn, _IO_default_xsgetn),
  JUMP_INIT(seekoff, _IO_file_seekoff),
  JUMP_INIT(seekpos, _IO_default_seekpos),
  JUMP_INIT(setbuf, _IO_file_setbuf),
  JUMP_INIT(sync, _IO_file_sync),
  JUMP_INIT(doallocate, _IO_file_doallocate),
  JUMP_INIT(read, _IO_cookie_read),
  JUMP_INIT(write, _IO_cookie_write),
  JUMP_INIT(seek, _IO_cookie_seek),
  JUMP_INIT(close, _IO_cookie_close),
  JUMP_INIT(stat, _IO_default_stat)
};


_IO_FILE *
fopencookie (cookie, mode, io_functions)
     void *cookie;
     const char *mode;
     _IO_cookie_io_functions_t io_functions;
{
  int read_write;
  struct locked_FILE
  {
    struct _IO_cookie_file cfile;
    _IO_lock_t lock;
  } *new_f;

  switch (*mode++)
    {
    case 'r':
      read_write = _IO_NO_WRITES;
      break;
    case 'w':
      read_write = _IO_NO_READS;
      break;
    case 'a':
      read_write = _IO_NO_READS|_IO_IS_APPENDING;
      break;
    default:
      return NULL;
  }
  if (mode[0] == '+' || (mode[0] == 'b' && mode[1] == '+'))
    read_write &= _IO_IS_APPENDING;

  new_f = (struct locked_FILE *) malloc (sizeof (struct locked_FILE));
  if (new_f == NULL)
    return NULL;
  new_f->cfile.file._lock = &new_f->lock;

  _IO_init (&new_f->cfile.file, 0);
  _IO_JUMPS (&new_f->cfile.file) = &_IO_cookie_jumps;
  new_f->cfile.cookie = cookie;
  new_f->cfile.io_functions = io_functions;

  _IO_file_init(&new_f->cfile.file);

  new_f->cfile.file._IO_file_flags =
    _IO_mask_flags (&new_f->cfile.file, read_write,
		    _IO_NO_READS+_IO_NO_WRITES+_IO_IS_APPENDING);

  return &new_f->cfile.file;
}
