/* Copyright (C) 2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* Dynamic module with TLS to be accessed by a signal handler to check safety
   of that mode. */

#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

/* This is an unlikely value to see in incorrectly initialized TLS
   block -- make sure we're initialized properly. */
static __thread intptr_t tls_data = 0xdeadbeef;

void
action (int signo, siginfo_t *info, void *ignored)
{
  if (tls_data != 0xdeadbeef)
    {
      write (STDOUT_FILENO, "wrong TLS value\n", 17);
      _exit (1);
    }

  /* arbitrary choice, just write something unique-ish. */
  tls_data = (intptr_t) info;
}
