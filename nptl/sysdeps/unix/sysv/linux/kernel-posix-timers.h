/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <pthread.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>


/* Nonzero if the system calls are not available.  */
extern int __no_posix_timers attribute_hidden;

/* Callback to start helper thread.  */
extern void __start_helper_thread (void) attribute_hidden;

/* Control variable for helper thread creation.  */
extern pthread_once_t __helper_once attribute_hidden;

/* TID of the helper thread.  */
extern pid_t __helper_tid attribute_hidden;


/* Type of timers in the kernel.  */
typedef int kernel_timer_t;


/* Internal representation of timer.  */
struct timer
{
  /* Notification mechanism.  */
  int sigev_notify;

  /* Timer ID returned by the kernel.  */
  kernel_timer_t ktimerid;

  /* All new elements must be added after ktimerid.  And if the thrfunc
     element is not the third element anymore the memory allocation in
     timer_create needs to be changed.  */

  /* Parameters for the thread to be started for SIGEV_THREAD.  */
  void (*thrfunc) (sigval_t);
  sigval_t sival;
  pthread_attr_t attr;
};
