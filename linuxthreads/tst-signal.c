/* Test sigaction wrapper.  */
/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@suse.de>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int
main (int argc, char *argv[])
{
  struct sigaction old_sa, new_sa;

  if (sigaction (SIGHUP, NULL, &old_sa) < 0)
    {
      printf ("cannot get signal action for SIGHUP: %m\n");
      exit (1);
    }

  if (old_sa.sa_handler != SIG_IGN)
    {
      printf ("SIGHUP action should be SIG_IGN, is %p\n",
	      (void *) old_sa.sa_handler);
      exit (1);
    }

  new_sa.sa_handler = SIG_DFL;
  if (sigaction (SIGHUP, &new_sa, NULL) < 0)
    {
      printf ("cannot set signal action for SIGHUP: %m\n");
      exit (1);
    }

  if (sigaction (SIGHUP, NULL, &old_sa) < 0)
    {
      printf ("cannot get signal action for SIGHUP: %m\n");
      exit (1);
    }

  if (old_sa.sa_handler != SIG_DFL)
    {
      printf ("SIGHUP action should be SIG_DFL, is %p\n",
	      (void *) old_sa.sa_handler);
      exit (1);
    }

  return 0;
}
