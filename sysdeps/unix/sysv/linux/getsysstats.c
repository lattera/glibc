/* Determine various system internal values, Linux version.
   Copyright (C) 1996-2001, 2002, 2003, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <alloca.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <mntent.h>
#include <paths.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#include <atomic.h>


/* How we can determine the number of available processors depends on
   the configuration.  There is currently (as of version 2.0.21) no
   system call to determine the number.  It is planned for the 2.1.x
   series to add this, though.

   One possibility to implement it for systems using Linux 2.0 is to
   examine the pseudo file /proc/cpuinfo.  Here we have one entry for
   each processor.

   But not all systems have support for the /proc filesystem.  If it
   is not available we simply return 1 since there is no way.  */

/* Other architectures use different formats for /proc/cpuinfo.  This
   provides a hook for alternative parsers.  */
#ifndef GET_NPROCS_PARSER
# define GET_NPROCS_PARSER(FP, BUFFER, RESULT)				\
  do									\
    {									\
      (RESULT) = 0;							\
      /* Read all lines and count the lines starting with the string	\
	 "processor".  We don't have to fear extremely long lines since	\
	 the kernel will not generate them.  8192 bytes are really	\
	 enough.  */							\
      while (fgets_unlocked (BUFFER, sizeof (BUFFER), FP) != NULL)	\
	if (strncmp (BUFFER, "processor", 9) == 0)			\
	  ++(RESULT);							\
    }									\
  while (0)
#endif

int
__get_nprocs ()
{
  char buffer[8192];
  int result = 1;

  /* XXX Here will come a test for the new system call.  */

  /* The /proc/stat format is more uniform, use it by default.  */
  FILE *fp = fopen ("/proc/stat", "rc");
  if (fp != NULL)
    {
      /* No threads use this stream.  */
      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      result = 0;
      while (fgets_unlocked (buffer, sizeof (buffer), fp) != NULL)
	if (strncmp (buffer, "cpu", 3) == 0 && isdigit (buffer[3]))
	  ++result;

      fclose (fp);
    }
  else
    {
      fp = fopen ("/proc/cpuinfo", "rc");
      if (fp != NULL)
	{
	  /* No threads use this stream.  */
	  __fsetlocking (fp, FSETLOCKING_BYCALLER);
	  GET_NPROCS_PARSER (fp, buffer, result);
	  fclose (fp);
	}
    }

  return result;
}
weak_alias (__get_nprocs, get_nprocs)


#ifdef GET_NPROCS_CONF_PARSER
/* On some architectures it is possible to distinguish between configured
   and active cpus.  */
int
__get_nprocs_conf ()
{
  char buffer[8192];
  int result = 1;

  /* XXX Here will come a test for the new system call.  */

  /* If we haven't found an appropriate entry return 1.  */
  FILE *fp = fopen ("/proc/cpuinfo", "rc");
  if (fp != NULL)
    {
      /* No threads use this stream.  */
      __fsetlocking (fp, FSETLOCKING_BYCALLER);
      GET_NPROCS_CONF_PARSER (fp, buffer, result);
      fclose (fp);
    }

  return result;
}
#else
/* As far as I know Linux has no separate numbers for configured and
   available processors.  So make the `get_nprocs_conf' function an
   alias.  */
strong_alias (__get_nprocs, __get_nprocs_conf)
#endif
weak_alias (__get_nprocs_conf, get_nprocs_conf)

/* General function to get information about memory status from proc
   filesystem.  */
static long int
internal_function
phys_pages_info (const char *format)
{
  char buffer[8192];
  long int result = -1;

  /* If we haven't found an appropriate entry return 1.  */
  FILE *fp = fopen ("/proc/meminfo", "rc");
  if (fp != NULL)
    {
      /* No threads use this stream.  */
      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      result = 0;
      /* Read all lines and count the lines starting with the
	 string "processor".  We don't have to fear extremely long
	 lines since the kernel will not generate them.  8192
	 bytes are really enough.  */
      while (fgets_unlocked (buffer, sizeof buffer, fp) != NULL)
	if (sscanf (buffer, format, &result) == 1)
	  {
	    result /= (__getpagesize () / 1024);
	    break;
	  }

      fclose (fp);
    }

  if (result == -1)
    /* We cannot get the needed value: signal an error.  */
    __set_errno (ENOSYS);

  return result;
}


/* Return the number of pages of physical memory in the system.  There
   is currently (as of version 2.0.21) no system call to determine the
   number.  It is planned for the 2.1.x series to add this, though.

   One possibility to implement it for systems using Linux 2.0 is to
   examine the pseudo file /proc/cpuinfo.  Here we have one entry for
   each processor.

   But not all systems have support for the /proc filesystem.  If it
   is not available we return -1 as an error signal.  */
long int
__get_phys_pages ()
{
  /* XXX Here will come a test for the new system call.  */

  return phys_pages_info ("MemTotal: %ld kB");
}
weak_alias (__get_phys_pages, get_phys_pages)


/* Return the number of available pages of physical memory in the
   system.  There is currently (as of version 2.0.21) no system call
   to determine the number.  It is planned for the 2.1.x series to add
   this, though.

   One possibility to implement it for systems using Linux 2.0 is to
   examine the pseudo file /proc/cpuinfo.  Here we have one entry for
   each processor.

   But not all systems have support for the /proc filesystem.  If it
   is not available we return -1 as an error signal.  */
long int
__get_avphys_pages ()
{
  /* XXX Here will come a test for the new system call.  */

  return phys_pages_info ("MemFree: %ld kB");
}
weak_alias (__get_avphys_pages, get_avphys_pages)
