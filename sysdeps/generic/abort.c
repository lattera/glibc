/* Copyright (C) 1991, 1993, 1995, 1996 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>


/* Function to close all streams and also make sure we don't loop by
   calling abort while closing the streams.  */
extern void __close_all_streams (void);


/* Cause an abnormal program termination with core-dump.  */
void
abort (void)
{
  struct sigaction act;
  sigset_t sigs;

  if (__sigemptyset (&sigs) == 0 &&
      __sigaddset (&sigs, SIGABRT) == 0)
    __sigprocmask (SIG_UNBLOCK, &sigs, (sigset_t *) NULL);

  /* If there is a user handler installed use it.  We don't close or
     flush streams.  */
  if (__sigaction (SIGABRT, NULL, &act) >= 0
      && act.sa_handler != SIG_DFL)
    {
      /* Send signal to call user handler.  */
      raise (SIGABRT);

      /* It returns, so we are responsible for closing the streams.  */
      __close_all_streams ();

      /* There was a handler installed.  Now remove it.  */
      memset (&act, '\0', sizeof (struct sigaction));
      act.sa_handler = SIG_DFL;
      __sigfillset (&act.sa_mask);
      act.sa_flags = 0;
      __sigaction (SIGABRT, &act, NULL);
    }
  else
    /* No handler installed so the next `raise' will hopefully
       terminate the process.  Therefore we must close the streams.  */
    __close_all_streams ();

  /* Try again.  */
  raise (SIGABRT);

  /* If we can't signal ourselves, exit.  */
  _exit (127);

  /* If even this fails make sure we never return.  */
  while (1)
    /* For ever and ever.  */
    ;
}
