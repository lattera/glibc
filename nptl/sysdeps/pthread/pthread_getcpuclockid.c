/* Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include <errno.h>
#include <pthreadP.h>
#include <sys/time.h>
#include <tls.h>


int
pthread_getcpuclockid (threadid, clockid)
     pthread_t threadid;
     clockid_t *clockid;
{
  struct pthread *pd = (struct pthread *) threadid;

  /* Make sure the descriptor is valid.  */
  if (INVALID_TD_P (pd))
    /* Not a valid thread handle.  */
    return ESRCH;

  /* We don't allow any process ID but our own.  */
  if (pd != THREAD_SELF)
    return EPERM;

#ifdef CLOCK_THREAD_CPUTIME_ID
  /* Store the number.  */
  *clockid = CLOCK_THREAD_CPUTIME_ID;

  return 0;
#else
  /* We don't have a timer for that.  */
  return ENOENT;
#endif
}
