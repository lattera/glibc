/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <signal.h>


/* If SET is not NULL, modify the current set of blocked signals
   according to HOW, which may be SIG_BLOCK, SIG_UNBLOCK or SIG_SETMASK.
   If OSET is not NULL, store the old set of blocked signals in *OSET.  */
int
DEFUN(__sigprocmask, (how, set, oset),
      int how AND CONST sigset_t *set AND sigset_t *oset)
{
  switch (how)
    {
    case SIG_BLOCK:
    case SIG_UNBLOCK:
    case SIG_SETMASK:
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  errno = ENOSYS;
  return -1;
}

/* No stub warning because abort calls __sigprocmask,
   and we don't want warnings for every use of abort on
   a system without safe signals.  */

weak_alias (__sigprocmask, sigprocmask)
