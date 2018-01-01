/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdio-lock.h>


int
__ftrylockfile (FILE *stream)
{
  return _IO_lock_trylock (*stream->_lock);
}
strong_alias (__ftrylockfile, _IO_ftrylockfile)
weak_alias (__ftrylockfile, ftrylockfile)
