/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1998 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* Spin locks */

#include <sched.h>
#include <time.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"

/* This function is called if the inlined test-and-set in acquire() failed */

/* The retry strategy is as follows:
   - We test and set the spinlock MAX_SPIN_COUNT times, calling
     sched_yield() each time.  This gives ample opportunity for other
     threads with priority >= our priority to make progress and
     release the spinlock.
   - If a thread with priority < our priority owns the spinlock,
     calling sched_yield() repeatedly is useless, since we're preventing
     the owning thread from making progress and releasing the spinlock.
     So, after MAX_SPIN_LOCK attemps, we suspend the calling thread
     using nanosleep().  This again should give time to the owning thread
     for releasing the spinlock.
     Notice that the nanosleep() interval must not be too small,
     since the kernel does busy-waiting for short intervals in a realtime
     process (!).  The smallest duration that guarantees thread
     suspension is currently 2ms.
   - When nanosleep() returns, we try again, doing MAX_SPIN_COUNT
     sched_yield(), then sleeping again if needed. */

void __pthread_acquire(int * spinlock)
{
  int cnt = 0;
  struct timespec tm;

  while (testandset(spinlock)) {
    if (cnt < MAX_SPIN_COUNT) {
      sched_yield();
      cnt++;
    } else {
      tm.tv_sec = 0;
      tm.tv_nsec = SPIN_SLEEP_DURATION;
      nanosleep(&tm, NULL);
      cnt = 0;
    }
  }
}
