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

#include <errno.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
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

    case _SC_ARG_MAX:
#ifdef	ARG_MAX
      return ARG_MAX;
#else
      return -1;
#endif

    case _SC_CHILD_MAX:
#ifdef	CHILD_MAX
      return CHILD_MAX;
#else
      return -1;
#endif

    case _SC_CLK_TCK:
#ifdef	CLK_TCK
      return CLK_TCK;
#else
      return 60;
#endif

    case _SC_NGROUPS_MAX:
#ifdef	NGROUPS_MAX
      return NGROUPS_MAX;
#else
      return -1;
#endif

    case _SC_OPEN_MAX:
      return __getdtablesize ();

    case _SC_STREAM_MAX:
#ifdef	STREAM_MAX
      return STREAM_MAX;
#else
      return FOPEN_MAX;
#endif

    case _SC_TZNAME_MAX:
      return __tzname_max ();

    case _SC_JOB_CONTROL:
#ifdef	_POSIX_JOB_CONTROL
      return 1;
#else
      return -1;
#endif

    case _SC_SAVED_IDS:
#ifdef	_POSIX_SAVED_IDS
      return 1;
#else
      return -1;
#endif

    case _SC_REALTIME_SIGNALS:
#ifdef	_POSIX_REALTIME_SIGNALS
      return 1;
#else
      return -1;
#endif

    case _SC_PRIORITY_SCHEDULING:
#ifdef	_POSIX_PRIORITY_SCHEDULING
      return 1;
#else
      return -1;
#endif

    case _SC_TIMERS:
#ifdef	_POSIX_TIMERS
      return 1;
#else
      return -1;
#endif

    case _SC_ASYNCHRONOUS_IO:
#ifdef	_POSIX_ASYNCHRONOUS_IO
      return 1;
#else
      return -1;
#endif

    case _SC_PRIORITIZED_IO:
#ifdef	_POSIX_PRIORITIZED_IO
      return 1;
#else
      return -1;
#endif

    case _SC_SYNCHRONIZED_IO:
#ifdef	_POSIX_SYNCHRONIZED_IO
      return 1;
#else
      return -1;
#endif

    case _SC_FSYNC:
#ifdef	_POSIX_FSYNC
      return 1;
#else
      return -1;
#endif

    case _SC_MAPPED_FILES:
#ifdef	_POSIX_MAPPED_FILES
      return 1;
#else
      return -1;
#endif

    case _SC_MEMLOCK:
#ifdef	_POSIX_MEMLOCK
      return 1;
#else
      return -1;
#endif

    case _SC_MEMLOCK_RANGE:
#ifdef	_POSIX_MEMLOCK_RANGE
      return 1;
#else
      return -1;
#endif

    case _SC_MEMORY_PROTECTION:
#ifdef	_POSIX_MEMORY_PROTECTION
      return 1;
#else
      return -1;
#endif

    case _SC_MESSAGE_PASSING:
#ifdef	_POSIX_MESSAGE_PASSING
      return 1;
#else
      return -1;
#endif

    case _SC_SEMAPHORES:
#ifdef	_POSIX_SEMAPHORES
      return 1;
#else
      return -1;
#endif

    case _SC_SHARED_MEMORY_OBJECTS:
#ifdef	_POSIX_SHARED_MEMORY_OBJECTS
      return 1;
#else
      return -1;
#endif

    case _SC_VERSION:
      return _POSIX_VERSION;

    case _SC_PAGESIZE:
      return __getpagesize ();

    case _SC_AIO_LISTIO_MAX:
#ifdef	AIO_LISTIO_MAX
      return AIO_LISTIO_MAX;
#else
      return -1;
#endif

    case _SC_AIO_MAX:
#ifdef	AIO_MAX
      return AIO_MAX;
#else
      return -1;
#endif

    case _SC_AIO_PRIO_DELTA_MAX:
#ifdef	AIO_PRIO_DELTA_MAX
      return AIO_PRIO_DELTA_MAX;
#else
      return -1;
#endif

    case _SC_DELAYTIMER_MAX:
#ifdef	DELAYTIMER_MAX
      return DELAYTIMER_MAX;
#else
      return -1;
#endif

    case _SC_MQ_OPEN_MAX:
#ifdef	MQ_OPEN_MAX
      return MQ_OPEN_MAX;
#else
      return -1;
#endif

    case _SC_MQ_PRIO_MAX:
#ifdef	MQ_PRIO_MAX
      return MQ_PRIO_MAX;
#else
      return -1;
#endif

    case _SC_RTSIG_MAX:
#ifdef	RTSIG_MAX
      return RTSIG_MAX;
#else
      return -1;
#endif

    case _SC_SEM_NSEMS_MAX:
#ifdef	SEM_NSEMS_MAX
      return SEM_NSEMS_MAX;
#else
      return -1;
#endif

    case _SC_SEM_VALUE_MAX:
#ifdef	SEM_VALUE_MAX
      return SEM_VALUE_MAX;
#else
      return -1;
#endif

    case _SC_SIGQUEUE_MAX:
#ifdef	SIGQUEUE_MAX
      return SIGQUEUE_MAX;
#else
      return -1;
#endif

    case _SC_TIMER_MAX:
#ifdef	TIMER_MAX
      return TIMER_MAX;
#else
      return -1;
#endif

    case _SC_BC_BASE_MAX:
#ifdef	BC_BASE_MAX
      return BC_BASE_MAX;
#else
      return -1;
#endif

    case _SC_BC_DIM_MAX:
#ifdef	BC_DIM_MAX
      return BC_DIM_MAX;
#else
      return -1;
#endif

    case _SC_BC_SCALE_MAX:
#ifdef	BC_SCALE_MAX
      return BC_SCALE_MAX;
#else
      return -1;
#endif

    case _SC_BC_STRING_MAX:
#ifdef	BC_STRING_MAX
      return BC_STRING_MAX;
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

    case _SC_EXPR_NEST_MAX:
#ifdef	EXPR_NEST_MAX
      return EXPR_NEST_MAX;
#else
      return -1;
#endif

    case _SC_LINE_MAX:
#ifdef	LINE_MAX
      return LINE_MAX;
#else
      return -1;
#endif

    case _SC_RE_DUP_MAX:
#ifdef	RE_DUP_MAX
      return RE_DUP_MAX;
#else
      return -1;
#endif

    case _SC_CHARCLASS_NAME_MAX:
#ifdef	CHARCLASS_NAME_MAX
      return CHARCLASS_NAME_MAX;
#else
      return -1;
#endif

    case _SC_PII:
#ifdef	_POSIX_PII
      return 1;
#else
      return -1;
#endif

    case _SC_PII_XTI:
#ifdef	_POSIX_PII_XTI
      return 1;
#else
      return -1;
#endif

    case _SC_PII_SOCKET:
#ifdef	_POSIX_PII_SOCKET
      return 1;
#else
      return -1;
#endif

    case _SC_PII_INTERNET:
#ifdef	_POSIX_PII_INTERNET
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI:
#ifdef	_POSIX_PII_OSI
      return 1;
#else
      return -1;
#endif

    case _SC_POLL:
#ifdef	_POSIX_POLL
      return 1;
#else
      return -1;
#endif

    case _SC_SELECT:
#ifdef	_POSIX_SELECT
      return 1;
#else
      return -1;
#endif

    case _SC_UIO_MAXIOV:
#ifdef	UIO_MAXIOV
      return UIO_MAXIOV;
#else
      return -1;
#endif

    case _SC_PII_INTERNET_STREAM:
#ifdef	_POSIX_PII_INTERNET_STREAM
      return 1;
#else
      return -1;
#endif

    case _SC_PII_INTERNET_DGRAM:
#ifdef	_POSIX_PII_INTERNET_DGRAM
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_COTS:
#ifdef	_POSIX_PII_OSI_COTS
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_CLTS:
#ifdef	_POSIX_PII_OSI_CLTS
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_M:
#ifdef	_POSIX_PII_OSI_M
      return 1;
#else
      return -1;
#endif

    case _SC_T_IOV_MAX:
#ifdef	_T_IOV_MAX
      return _T_IOV_MAX;
#else
      return -1;
#endif

    case _SC_2_VERSION:
      /* This is actually supposed to return the version
	 of the 1003.2 utilities on the system {POSIX2_VERSION}.  */
      return _POSIX2_C_VERSION;

    case _SC_2_C_BIND:
#ifdef	_POSIX2_C_BIND
      return _POSIX2_C_BIND;
#else
      return -1;
#endif

    case _SC_2_C_DEV:
#ifdef	_POSIX2_C_DEV
      return _POSIX2_C_DEV;
#else
      return -1;
#endif

    case _SC_2_C_VERSION:
#ifdef	_POSIX2_C_VERSION
      return _POSIX2_C_VERSION;
#else
      return -1;
#endif

    case _SC_2_FORT_DEV:
#ifdef	_POSIX2_FORT_DEV
      return _POSIX2_FORT_DEV;
#else
      return -1;
#endif

    case _SC_2_LOCALEDEF:
#ifdef	_POSIX2_LOCALEDEF
      return _POSIX2_LOCALEDEF;
#else
      return -1;
#endif

    case _SC_2_SW_DEV:
#ifdef	_POSIX2_SW_DEV
      return _POSIX2_SW_DEV;
#else
      return -1;
#endif

    case _SC_2_CHAR_TERM:
#ifdef	_POSIX2_CHAR_TERM
      return _POSIX2_CHAR_TERM;
#else
      return -1;
#endif

    case _SC_2_UPE:
#ifdef	_POSIX2_UPE
      return _POSIX2_UPE;
#else
      return -1;
#endif

      /* POSIX 1003.1c (POSIX Threads).  */
    case _SC_THREADS:
#ifdef	_POSIX_THREADS
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_SAFE_FUNCTIONS:
#ifdef	_POSIX_THREAD_SAFE_FUNCTIONS
      return 1;
#else
      return -1;
#endif

    case _SC_GETGR_R_SIZE_MAX:
#ifdef	NSS_BUFLEN_GROUP
      return NSS_BUFLEN_GROUP;
#else
      return -1;
#endif

    case _SC_GETPW_R_SIZE_MAX:
#ifdef	NSS_BUFLEN_PASSWD
      return NSS_BUFLEN_PASSWD;
#else
      return -1;
#endif

    case _SC_LOGIN_NAME_MAX:
#ifdef	_POSIX_LOGIN_NAME_MAX
      return _POSIX_LOGIN_NAME_MAX;
#else
      return -1;
#endif

    case _SC_TTY_NAME_MAX:
#ifdef	_POSIX_TTY_NAME_MAX
      return _POSIX_TTY_NAME_MAX;
#else
      return -1;
#endif

    case _SC_THREAD_DESTRUCTOR_ITERATIONS:
#ifdef	_POSIX_THREAD_DESTRUCTOR_ITERATIONS
      return _POSIX_THREAD_DESTRUCTOR_ITERATIONS;
#else
      return -1;
#endif

    case _SC_THREAD_KEYS_MAX:
#ifdef	PTHREAD_KEYS_MAX
      return PTHREAD_KEYS_MAX;
#else
      return -1;
#endif

    case _SC_THREAD_STACK_MIN:
#ifdef	PTHREAD_STACK_MIN
      return PTHREAD_STACK_MIN;
#else
      return -1;
#endif

    case _SC_THREAD_THREADS_MAX:
#ifdef	PTHREAD_THREADS_MAX
      return PTHREAD_THREADS_MAX;
#else
      return -1;
#endif

    case _SC_THREAD_ATTR_STACKADDR:
#ifdef	_POSIX_THREAD_ATTR_STACKADDR
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_ATTR_STACKSIZE:
#ifdef	_POSIX_THREAD_ATTR_STACKSIZE
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_PRIORITY_SCHEDULING:
#ifdef	_POSIX_THREAD_PRIORITY_SCHEDULING
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_PRIO_INHERIT:
#ifdef	_POSIX_THREAD_PRIO_INHERIT
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_PRIO_PROTECT:
#ifdef	_POSIX_THREAD_PRIO_PROTECT
      return 1;
#else
      return -1;
#endif

    case _SC_THREAD_PROCESS_SHARED:
#ifdef	_POSIX_THREAD_PROCESS_SHARED
      return 1;
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

    case _SC_XOPEN_VERSION:
      return _XOPEN_VERSION;

    case _SC_XOPEN_XCU_VERSION:
      return _XOPEN_XCU_VERSION;

    case _SC_XOPEN_UNIX:
      return _XOPEN_UNIX;

    case _SC_XOPEN_CRYPT:
#ifdef	_XOPEN_CRYPT
      return _XOPEN_CRYPT;
#else
      return -1;
#endif

    case _SC_XOPEN_ENH_I18N:
#ifdef	_XOPEN_ENH_I18N
      return _XOPEN_ENH_I18N;
#else
      return -1;
#endif

    case _SC_XOPEN_SHM:
#ifdef	_XOPEN_SHM
      return _XOPEN_SHM;
#else
      return -1;
#endif

    case _SC_XOPEN_XPG2:
#ifdef	_XOPEN_XPG2
      return _XOPEN_XPG2;
#else
      return -1;
#endif

    case _SC_XOPEN_XPG3:
#ifdef	_XOPEN_XPG3
      return _XOPEN_XPG3;
#else
      return -1;
#endif

    case _SC_XOPEN_XPG4:
#ifdef	_XOPEN_XPG4
      return _XOPEN_XPG4;
#else
      return -1;
#endif

    case _SC_CHAR_BIT:
      return CHAR_BIT;

    case _SC_CHAR_MAX:
      return CHAR_MAX;

    case _SC_CHAR_MIN:
      return CHAR_MIN;

    case _SC_INT_MAX:
      return INT_MAX;

    case _SC_INT_MIN:
      return INT_MIN;

    case _SC_LONG_BIT:
      return sizeof (long int) * CHAR_BIT;

    case _SC_WORD_BIT:
      return sizeof (int) * CHAR_BIT;

    case _SC_MB_LEN_MAX:
      return MB_LEN_MAX;

    case _SC_NZERO:
      return NZERO;

    case _SC_SSIZE_MAX:
      return _POSIX_SSIZE_MAX;

    case _SC_SCHAR_MAX:
      return SCHAR_MAX;

    case _SC_SCHAR_MIN:
      return SCHAR_MIN;

    case _SC_SHRT_MAX:
      return SHRT_MAX;

    case _SC_SHRT_MIN:
      return SHRT_MIN;

    case _SC_UCHAR_MAX:
      return UCHAR_MAX;

    case _SC_UINT_MAX:
      return UINT_MAX;

    case _SC_ULONG_MAX:
      return ULONG_MAX;

    case _SC_USHRT_MAX:
      return USHRT_MAX;

    case _SC_NL_ARGMAX:
#ifdef	NL_ARGMAX
      return NL_ARGMAX;
#else
      return -1;
#endif

    case _SC_NL_LANGMAX:
#ifdef	NL_LANGMAX
      return NL_LANGMAX;
#else
      return -1;
#endif

    case _SC_NL_MSGMAX:
#ifdef	NL_MSGMAX
      return NL_MSGMAX;
#else
      return -1;
#endif

    case _SC_NL_NMAX:
#ifdef	NL_NMAX
      return NL_NMAX;
#else
      return -1;
#endif

    case _SC_NL_SETMAX:
#ifdef	NL_SETMAX
      return NL_SETMAX;
#else
      return -1;
#endif

    case _SC_NL_TEXTMAX:
#ifdef	NL_TEXTMAX
      return NL_TEXTMAX;
#else
      return -1;
#endif
    }
}

weak_alias (__sysconf, sysconf)
