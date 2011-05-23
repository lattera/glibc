/* Get file-specific information about a file.  Linux version.
   Copyright (C) 2003,2004,2006 2008,2009,2011 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stdlib.h>
#include <sysdep.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <not-cancel.h>
#include <ldsodefs.h>

/* Legacy value of ARG_MAX.  The macro is now not defined since the
   actual value varies based on the stack size.  */
#define legacy_ARG_MAX 131072


static long int posix_sysconf (int name);


#ifndef HAS_CPUCLOCK
static long int
has_cpuclock (int name)
{
# if defined __NR_clock_getres || HP_TIMING_AVAIL
  /* If we have HP_TIMING, we will fall back on that if the system
     call does not work, so we support it either way.  */
#  if !HP_TIMING_AVAIL
  /* Check using the clock_getres system call.  */
  struct timespec ts;
  INTERNAL_SYSCALL_DECL (err);
  int r = INTERNAL_SYSCALL (clock_getres, err, 2,
			    (name == _SC_CPUTIME
			     ? CLOCK_PROCESS_CPUTIME_ID
			     : CLOCK_THREAD_CPUTIME_ID),
			    &ts);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return -1;
#  endif
  return _POSIX_VERSION;
# else
  return -1;
# endif
}
# define HAS_CPUCLOCK(name) has_cpuclock (name)
#endif


/* Get the value of the system variable NAME.  */
long int
__sysconf (int name)
{
  const char *procfname = NULL;

  switch (name)
    {
      struct rlimit rlimit;
#ifdef __NR_clock_getres
    case _SC_MONOTONIC_CLOCK:
      /* Check using the clock_getres system call.  */
      {
	struct timespec ts;
	INTERNAL_SYSCALL_DECL (err);
	int r;
	r = INTERNAL_SYSCALL (clock_getres, err, 2, CLOCK_MONOTONIC, &ts);
	return INTERNAL_SYSCALL_ERROR_P (r, err) ? -1 : _POSIX_VERSION;
      }
#endif

    case _SC_CPUTIME:
    case _SC_THREAD_CPUTIME:
      return HAS_CPUCLOCK (name);

    case _SC_ARG_MAX:
#if __LINUX_KERNEL_VERSION < 0x020617
      /* Determine whether this is a kernel 2.6.23 or later.  Only
	 then do we have an argument limit determined by the stack
	 size.  */
      if (GLRO(dl_discover_osversion) () >= 0x020617)
#endif
	/* Use getrlimit to get the stack limit.  */
	if (__getrlimit (RLIMIT_STACK, &rlimit) == 0)
	  return MAX (legacy_ARG_MAX, rlimit.rlim_cur / 4);

      return legacy_ARG_MAX;

    case _SC_NGROUPS_MAX:
      /* Try to read the information from the /proc/sys/kernel/ngroups_max
	 file.  */
      procfname = "/proc/sys/kernel/ngroups_max";
      break;

    case _SC_SIGQUEUE_MAX:
      if (__getrlimit (RLIMIT_SIGPENDING, &rlimit) == 0)
	return rlimit.rlim_cur;

      /* The /proc/sys/kernel/rtsig-max file contains the answer.  */
      procfname = "/proc/sys/kernel/rtsig-max";
      break;

    default:
      break;
    }

  if (procfname != NULL)
    {
      int fd = open_not_cancel_2 (procfname, O_RDONLY);
      if (fd != -1)
	{
	  /* This is more than enough, the file contains a single integer.  */
	  char buf[32];
	  ssize_t n;
	  n = TEMP_FAILURE_RETRY (read_not_cancel (fd, buf, sizeof (buf) - 1));
	  close_not_cancel_no_status (fd);

	  if (n > 0)
	    {
	      /* Terminate the string.  */
	      buf[n] = '\0';

	      char *endp;
	      long int res = strtol (buf, &endp, 10);
	      if (endp != buf && (*endp == '\0' || *endp == '\n'))
		return res;
	    }
	}
    }

  return posix_sysconf (name);
}

/* Now the POSIX version.  */
#undef __sysconf
#define __sysconf static posix_sysconf
#include <sysdeps/posix/sysconf.c>
