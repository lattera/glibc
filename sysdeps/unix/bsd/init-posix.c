/* Copyright (C) 1991, 1994, 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <time.h>

/* This must be initialized data or its presence will not be sufficient to
   merit linkage of this file, which is necessary for the real
   initialization function below to be called.  */
time_t _posix_start_time = -1;

void
DEFUN_VOID(__init_posix)
{
  _posix_start_time = time((time_t *) NULL);
}

#ifdef	HAVE_GNU_LD
text_set_element(__libc_subinit, __init_posix);
#endif
