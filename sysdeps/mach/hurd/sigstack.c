/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <hurd.h>

/* Run signals handlers on the stack specified by SS (if not NULL).
   If OSS is not NULL, it is filled in with the old signal stack status.  */
int
DEFUN(sigstack, (ss, oss),
      CONST struct sigstack *ss AND struct sigstack *oss)
{
  struct sigaltstack as, oas;

  as.ss_sp = ss->ss_sp;
  as.ss_size = 0;
  as.ss_flags = 0;

  if (sigaltstack (&as, &oas) < 0)
    return -1;

  if (oss != NULL)
    {
      oss->ss_sp = oas.ss_sp;
      oss->ss_onstack = oas.ss_flags & SA_ONSTACK;
    }

  return 0;
}
