/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kaz Kylheku <kaz@ashi.footprints.net>.

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

#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "posix-timer.h"


/* Set timer TIMERID to VALUE, returning old value in OVLAUE.  */
int
timer_settime (timerid, flags, value, ovalue)
     timer_t timerid;
     int flags;
     const struct itimerspec *value;
     struct itimerspec *ovalue;
{
  struct timer_node *timer;
  struct thread_node *thread = NULL;
  struct timespec now;
  int have_now = 0;
  int retval = -1;

  pthread_mutex_lock (&__timer_mutex);

  timer = timer_id2ptr (timerid);
  if (timer == NULL)
    {
      errno = EINVAL;
      goto bail;
    }

  if (value->it_interval.tv_nsec < 0
      || value->it_interval.tv_nsec >= 1000000000
      || value->it_value.tv_nsec < 0
      || value->it_value.tv_nsec >= 1000000000)
    {
      errno = EINVAL;
      goto bail;
    }

  if (ovalue != NULL)
    {
      ovalue->it_interval = timer->value.it_interval;

      if (timer->armed)
	{
	  clock_gettime (timer->clock, &now);
	  have_now = 1;
	  timespec_sub (&ovalue->it_value, &timer->expirytime, &now);
	}
      else
	{
	  ovalue->it_value.tv_sec = 0;
	  ovalue->it_value.tv_nsec = 0;
	}
    }

  timer->value = *value;

  list_unlink (&timer->links);
  timer->armed = 0;

  thread = timer->thread;

  if (value->it_value.tv_sec != 0
      || __builtin_expect (value->it_value.tv_nsec != 0, 1))
    {
      if ((flags & TIMER_ABSTIME) != 0)
	/* The user specified the expiration time.  */
	timer->expirytime = value->it_value;
      else
	{
	  if (! have_now)
	    clock_gettime (timer->clock, &now);

	  timespec_add (&timer->expirytime, &now, &value->it_value);
        }

      __timer_thread_queue_timer (thread, timer);
      timer->armed = 1;
    }

  retval = 0;

bail:
  pthread_mutex_unlock (&__timer_mutex);

  /* TODO: optimize this. Only need to wake up the thread if inserting
     a new timer at the head of the queue.  */
  if (thread != NULL)
    __timer_thread_wakeup (thread);

  return retval;
}
