/* Close all streams but make sure this isn't done more than once.
   This function is called in abort().
   Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <libc-lock.h>
#include <stdio.h>

static int already_called;

__libc_lock_define_initialized (static, lock);

void
__close_all_streams (void)
{
  /* We must be prepared for multi-threading on multiple calls.  */
  if (! __libc_lock_trylock (lock) && ! already_called)
    {
      /* Signal that we already did this.  */
      already_called = 1;

      /* Do the real work.  */
      fclose (NULL);

      /* We don't release the lock so that the `trylock' immediately
	 fails.  */
    }
}
