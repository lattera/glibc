/* Copyright (C) 2002-2013 Free Software Foundation, Inc.
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


static int current_rtmin = __SIGRTMIN + 2;
static int current_rtmax = __SIGRTMAX;


/* We reserve __SIGRTMIN for use as the cancelation signal.  This
   signal is used internally.  */
int
__libc_current_sigrtmin (void)
{
  return current_rtmin;
}
libc_hidden_def (__libc_current_sigrtmin)
strong_alias (__libc_current_sigrtmin, __libc_current_sigrtmin_private)


int
__libc_current_sigrtmax (void)
{
  return current_rtmax;
}
libc_hidden_def (__libc_current_sigrtmax)
strong_alias (__libc_current_sigrtmax, __libc_current_sigrtmax_private)


int
__libc_allocate_rtsig (int high)
{
  if (current_rtmin == -1 || current_rtmin > current_rtmax)
    /* We don't have anymore signal available.  */
    return -1;

  return high ? current_rtmin++ : current_rtmax--;
}
strong_alias (__libc_allocate_rtsig, __libc_allocate_rtsig_private)
