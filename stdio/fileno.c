/* Copyright (C) 1991, 93, 94, 96, 97, 98 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdio.h>

/* Return the system file descriptor associated with STREAM.  */
int
fileno (stream)
     FILE *stream;
{
  extern void __stdio_check_funcs __P ((FILE *));

  if (! __validfp (stream))
    {
      __set_errno (EINVAL);
      return -1;
    }

  __stdio_check_funcs (stream);

  if (stream->__io_funcs.__fileno == NULL)
    {
#ifdef EOPNOTSUPP
      __set_errno (EOPNOTSUPP);
#else
      __set_errno (ENOSYS);
#endif
      return -1;
    }

  return (*stream->__io_funcs.__fileno) (stream->__cookie);
}

weak_alias(fileno, fileno_unlocked)
