/* Copyright (C) 2000, 2001, 2004 Free Software Foundation, Inc.
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
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <internals.h>

int
pthread_getcpuclockid (pthread_t thread_id, clockid_t *clock_id)
{
#ifdef CLOCK_THREAD_CPUTIME_ID
  /* We need to store the thread ID in the CLOCKID variable together
     with a number identifying the clock.  We reserve the low 3 bits
     for the clock ID and the rest for the thread ID.  This is
     problematic if the thread ID is too large.  But 29 bits should be
     fine.

     If some day more clock IDs are needed the ID part can be
     enlarged.  The IDs are entirely internal.  */
  if (2 * PTHREAD_THREADS_MAX
      >= 1 << (8 * sizeof (*clock_id) - CLOCK_IDFIELD_SIZE))
    return ERANGE;

  /* Store the number.  */
  *clock_id = CLOCK_THREAD_CPUTIME_ID | (thread_id << CLOCK_IDFIELD_SIZE);

  return 0;
#else
  /* We don't have a timer for that.  */
  return ENOENT;
#endif
}
