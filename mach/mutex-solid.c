/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <lock-intern.h>
#include <cthreads.h>

/* If cthreads is linked in, it will define these variables with values
   that point to its mutex functions.  */
void (*_cthread_mutex_lock_routine) (struct mutex *);
void (*_cthread_mutex_unlock_routine) (struct mutex *);

void
__mutex_lock_solid (void *lock)
{
  if (_cthread_mutex_lock_routine)
    (*_cthread_mutex_lock_routine) (lock);
  else
    __spin_lock_solid (lock);
}

void
__mutex_unlock_solid (void *lock)
{
  if (_cthread_mutex_unlock_routine)
    (*_cthread_mutex_unlock_routine) (lock);
}

void
__mutex_init (void *lock)
{
  /* This happens to be name space-safe because it is a macro.
     It invokes only spin_lock_init, which is a macro for __spin_lock_init;
     and cthread_queue_init, which is a macro for some simple code.  */
  mutex_init ((struct mutex *) lock);
}
