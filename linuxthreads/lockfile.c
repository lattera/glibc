/* lockfile - Handle locking and unlocking of stream.
   Copyright (C) 1996, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <bits/libc-lock.h>
#include <stdio.h>
#include <pthread.h>
#include "internals.h"
#include "../libio/libioP.h"

void
__flockfile (FILE *stream)
{
  __pthread_mutex_lock (stream->_lock);
}
#undef _IO_flockfile
strong_alias (__flockfile, _IO_flockfile)
weak_alias (__flockfile, flockfile);


void
__funlockfile (FILE *stream)
{
  __pthread_mutex_unlock (stream->_lock);
}
#undef _IO_funlockfile
strong_alias (__funlockfile, _IO_funlockfile)
weak_alias (__funlockfile, funlockfile);


int
__ftrylockfile (FILE *stream)
{
  return __pthread_mutex_trylock (stream->_lock);
}
strong_alias (__ftrylockfile, _IO_ftrylockfile)
weak_alias (__ftrylockfile, ftrylockfile);

void
__flockfilelist(void)
{
  _IO_list_lock();
}

void
__funlockfilelist(void)
{
  _IO_list_unlock();
}

void
__fresetlockfiles (void)
{
  _IO_ITER i;

  pthread_mutexattr_t attr;

  __pthread_mutexattr_init (&attr);
  __pthread_mutexattr_settype (&attr, PTHREAD_MUTEX_RECURSIVE_NP);

  for (i = _IO_iter_begin(); i != _IO_iter_end(); i = _IO_iter_next(i))
    __pthread_mutex_init (_IO_iter_file(i)->_lock, &attr);

  __pthread_mutexattr_destroy (&attr);

  _IO_list_resetlock();
}
