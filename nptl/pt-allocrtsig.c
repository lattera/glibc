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

#include <signal.h>


/* These are defined in libc.  We want to have only one definition
   so we "forward" the calls.  */
extern int __libc_current_sigrtmin_private (void);
extern int __libc_current_sigrtmax_private (void);
extern int __libc_allocate_rtsig_private (int high);


/* We reserve __SIGRTMIN for use as the cancellation signal and
   __SIGRTMIN+1 to handle setuid et.al.  These signals are used
   internally.  */
int
__libc_current_sigrtmin (void)
{
  return __libc_current_sigrtmin_private ();
}


int
__libc_current_sigrtmax (void)
{
  return __libc_current_sigrtmax_private ();
}


int
__libc_allocate_rtsig (int high)
{
  return __libc_allocate_rtsig_private (high);
}
