/* lockfile - Handle locking and unlocking of stream.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

typedef FILE;


void
__internal_flockfile (FILE *stream)
{
  /* Do nothing.  Using this version does not do any locking.  */
}
#ifdef USE_IN_LIBIO
weak_alias (__internal_flockfile, _IO_flockfile)
#else
weak_alias (__internal_flockfile, __flockfile)
#endif
weak_alias (__internal_flockfile, flockfile);


void
__internal_funlockfile (FILE *stream)
{
  /* Do nothing.  Using this version does not do any locking.  */
}
#ifdef USE_IN_LIBIO
weak_alias (__internal_funlockfile, _IO_funlockfile)
#else
weak_alias (__internal_funlockfile, __internal_funlockfile)
#endif
weak_alias (__internal_funlockfile, funlockfile);


int
__internal_ftrylockfile (FILE *stream)
{
  /* Do nothing.  Using this version does not do any locking.  */
  return 1;
}
#ifdef USE_IN_LIBIO
weak_alias (__internal_ftrylockfile, __ftrylockfile)
#else
weak_alias (__internal_ftrylockfile, _IO_ftrylockfile)
#endif
weak_alias (__internal_ftrylockfile, ftrylockfile);
