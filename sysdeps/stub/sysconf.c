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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/sysinfo.h>


/* Get the value of the system variable NAME.  */
long int
__sysconf (name)
     int name;
{
  switch (name)
    {
    default:
      __set_errno (EINVAL);
      return -1;

    case _SC_TZNAME_MAX:
      return __tzname_max ();

    case _SC_CHARCLASS_NAME_MAX:
#ifdef	CHARCLASS_NAME_MAX
      return CHARCLASS_NAME_MAX;
#else
      return -1;
#endif

    case _SC_COLL_WEIGHTS_MAX:
#ifdef	COLL_WEIGHTS_MAX
      return COLL_WEIGHTS_MAX;
#else
      return -1;
#endif

    case _SC_EQUIV_CLASS_MAX:
#ifdef	EQUIV_CLASS_MAX
      return EQUIV_CLASS_MAX;
#else
      return -1;
#endif

    case _SC_2_LOCALEDEF:
#ifdef	_POSIX2_LOCALEDEF
      return _POSIX2_LOCALEDEF;
#else
      return -1;
#endif

    case _SC_NPROCESSORS_CONF:
      return __get_nprocs_conf ();

    case _SC_NPROCESSORS_ONLN:
      return __get_nprocs ();

    case _SC_PHYS_PAGES:
      return __get_phys_pages ();

    case _SC_AVPHYS_PAGES:
      return __get_avphys_pages ();

    case _SC_ATEXIT_MAX:
      /* We have no limit since we use lists.  */
      return INT_MAX;

    case _SC_PASS_MAX:
      /* We have no limit but since the return value might be used to
	 allocate a buffer we restrict the value.  */
      return BUFSIZ;

    case _SC_ARG_MAX:
    case _SC_CHILD_MAX:
    case _SC_CLK_TCK:
    case _SC_NGROUPS_MAX:
    case _SC_OPEN_MAX:
    case _SC_STREAM_MAX:
    case _SC_JOB_CONTROL:
    case _SC_SAVED_IDS:
    case _SC_REALTIME_SIGNALS:
    case _SC_PRIORITY_SCHEDULING:
    case _SC_TIMERS:
    case _SC_ASYNCHRONOUS_IO:
    case _SC_PRIORITIZED_IO:
    case _SC_SYNCHRONIZED_IO:
    case _SC_FSYNC:
    case _SC_MAPPED_FILES:
    case _SC_MEMLOCK:
    case _SC_MEMLOCK_RANGE:
    case _SC_MEMORY_PROTECTION:
    case _SC_MESSAGE_PASSING:
    case _SC_SEMAPHORES:
    case _SC_SHARED_MEMORY_OBJECTS:

    case _SC_AIO_LIST_MAX:
    case _SC_AIO_MAX:
    case _SC_AIO_PRIO_DELTA_MAX:
    case _SC_DELAYTIME_MAX:
    case _SC_MQ_OPEN_MAX:
    case _SC_MQ_PRIO_MAX:
    case _SC_VERSION:
    case _SC_PAGESIZE:
    case _SC_RTSIG_MAX:
    case _SC_SEM_NSEMS_MAX:
    case _SC_SEM_VALUE_MAX:
    case _SC_SIGQUEUE_MAX:
    case _SC_TIMER_MAX:

    case _SC_PII:
    case _SC_PII_XTI:
    case _SC_PII_SOCKET:
    case _SC_PII_OSI:
    case _SC_POLL:
    case _SC_SELECT:
    case _SC_UIO_MAXIOV:
    case _SC_PII_INTERNET_STREAM:
    case _SC_PII_INTERNET_DGRAM:
    case _SC_PII_OSI_COTS:
    case _SC_PII_OSI_CLTS:
    case _SC_PII_OSI_M:
    case _SC_T_IOV_MAX:

    case _SC_BC_BASE_MAX:
    case _SC_BC_DIM_MAX:
    case _SC_BC_SCALE_MAX:
    case _SC_BC_STRING_MAX:
    case _SC_EXPR_NEST_MAX:
    case _SC_LINE_MAX:
    case _SC_RE_DUP_MAX:
    case _SC_2_VERSION:
    case _SC_2_C_BIND:
    case _SC_2_C_DEV:
    case _SC_2_FORT_DEV:
    case _SC_2_SW_DEV:

    case _SC_THREADS:
    case _SC_THREAD_SAFE_FUNCTIONS:
    case _SC_GETGR_R_SIZE_MAX:
    case _SC_GETPW_R_SIZE_MAX:
    case _SC_LOGIN_NAME_MAX:
    case _SC_TTY_NAME_MAX:
    case _SC_THREAD_DESTRUCTOR_ITERATIONS:
    case _SC_THREAD_KEYS_MAX:
    case _SC_THREAD_STACK_MIN:
    case _SC_THREAD_THREADS_MAX:
    case _SC_THREAD_ATTR_STACKADDR:
    case _SC_THREAD_ATTR_STACKSIZE:
    case _SC_THREAD_PRIORITY_SCHEDULING:
    case _SC_THREAD_PRIO_INHERIT:
    case _SC_THREAD_PRIO_PROTECT:
    case _SC_THREAD_PROCESS_SHARED:

    case _SC_XOPEN_VERSION:

      break;
    }

  __set_errno (ENOSYS);
  return -1;
}

weak_alias (__sysconf, sysconf)

stub_warning (sysconf)
