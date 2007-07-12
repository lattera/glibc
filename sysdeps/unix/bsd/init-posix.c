/* Copyright (C) 1991, 94, 95, 97, 98, 2005 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <time.h>

/* This must be initialized data or its presence will not be sufficient to
   merit linkage of this file, which is necessary for the real
   initialization function below to be called.  */
time_t _posix_start_time = -1;

void
__init_posix (void)
{
  _posix_start_time = time ((time_t *) NULL);
}

text_set_element(__libc_subinit, __init_posix);
