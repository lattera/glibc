/* Copyright (C) 2000, 2001, 2003, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#include "has_cpuclock.c"


int
clock_getcpuclockid (pid_t pid, clockid_t *clock_id)
{
  /* We don't allow any process ID but our own.  */
  if (pid != 0 && pid != getpid ())
    return EPERM;

  int retval = ENOENT;

  if (has_cpuclock () > 0)
    {
      /* Store the number.  */
      *clock_id = CLOCK_PROCESS_CPUTIME_ID;
      retval = 0;
    }

  return retval;
}
