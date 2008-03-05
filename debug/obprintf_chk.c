/* Print output of stream to given obstack.
   Copyright (C) 1996,1997,1999,2000,2001,2002,2003,2004,2005,2006,2008
	Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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


#include <stdlib.h>
#include <libioP.h>
#include "../libio/strfile.h"
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <obstack.h>
#include <stdarg.h>
#include <stdio_ext.h>


struct _IO_obstack_file
{
  struct _IO_FILE_plus file;
  struct obstack *obstack;
};

extern const struct _IO_jump_t _IO_obstack_jumps attribute_hidden;

int
__obstack_vprintf_chk (struct obstack *obstack, int flags, const char *format,
		       va_list args)
{
  struct obstack_FILE
    {
      struct _IO_obstack_file ofile;
    } new_f;
  int result;
  int size;
  int room;

#ifdef _IO_MTSAFE_IO
  new_f.ofile.file.file._lock = NULL;
#endif

  _IO_no_init (&new_f.ofile.file.file, _IO_USER_LOCK, -1, NULL, NULL);
  _IO_JUMPS (&new_f.ofile.file) = &_IO_obstack_jumps;
  room = obstack_room (obstack);
  size = obstack_object_size (obstack) + room;
  if (size == 0)
    {
      /* We have to handle the allocation a bit different since the
	 `_IO_str_init_static' function would handle a size of zero
	 different from what we expect.  */

      /* Get more memory.  */
      obstack_make_room (obstack, 64);

      /* Recompute how much room we have.  */
      room = obstack_room (obstack);
      size = room;

      assert (size != 0);
    }

  _IO_str_init_static_internal ((struct _IO_strfile_ *) &new_f.ofile,
				obstack_base (obstack),
				size, obstack_next_free (obstack));
  /* Now allocate the rest of the current chunk.  */
  assert (size == (new_f.ofile.file.file._IO_write_end
		   - new_f.ofile.file.file._IO_write_base));
  assert (new_f.ofile.file.file._IO_write_ptr
	  == (new_f.ofile.file.file._IO_write_base
	      + obstack_object_size (obstack)));
  obstack_blank_fast (obstack, room);

  new_f.ofile.obstack = obstack;

  /* For flags > 0 (i.e. __USE_FORTIFY_LEVEL > 1) request that %n
     can only come from read-only format strings.  */
  if (flags > 0)
    new_f.ofile.file.file._flags2 |= _IO_FLAGS2_FORTIFY;

  result = INTUSE(_IO_vfprintf) (&new_f.ofile.file.file, format, args);

  /* Shrink the buffer to the space we really currently need.  */
  obstack_blank_fast (obstack, (new_f.ofile.file.file._IO_write_ptr
				- new_f.ofile.file.file._IO_write_end));

  return result;
}
libc_hidden_def (__obstack_vprintf_chk)


int
__obstack_printf_chk (struct obstack *obstack, int flags, const char *format,
		      ...)
{
  int result;
  va_list ap;
  va_start (ap, format);
  result = __obstack_vprintf_chk (obstack, flags, format, ap);
  va_end (ap);
  return result;
}
