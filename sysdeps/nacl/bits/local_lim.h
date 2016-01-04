/* Minimum guaranteed maximum values for system limits.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define NAME_MAX        255

#define PATH_MAX        4096

#define NGROUPS_MAX    65536

/* The number of data keys per process.  */
#define _POSIX_THREAD_KEYS_MAX  128
/* This is the value this implementation supports.  */
#define PTHREAD_KEYS_MAX        1024

/* Controlling the iterations of destructors for thread-specific data.  */
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS     4
/* Number of iterations this implementation does.  */
#define PTHREAD_DESTRUCTOR_ITERATIONS   _POSIX_THREAD_DESTRUCTOR_ITERATIONS

/* The number of threads per process.  */
#define _POSIX_THREAD_THREADS_MAX       64
/* We have no predefined limit on the number of threads.  */
#undef PTHREAD_THREADS_MAX

/* Maximum amount by which a process can descrease its asynchronous I/O
   priority level.  */
#define AIO_PRIO_DELTA_MAX      20

/* Minimum size for a thread.  We are free to choose a reasonable value.  */
#define PTHREAD_STACK_MIN       131072

/* Maximum number of timer expiration overruns.  */
#define DELAYTIMER_MAX          2147483647

/* Maximum tty name length.  */
#define TTY_NAME_MAX            32

/* Maximum login name length.  This is arbitrary.  */
#define LOGIN_NAME_MAX          256

/* Maximum host name length.  */
#define HOST_NAME_MAX           64

/* Maximum message queue priority level.  */
#define MQ_PRIO_MAX             32768

/* Maximum value the semaphore can have.  */
#define SEM_VALUE_MAX           (2147483647)
