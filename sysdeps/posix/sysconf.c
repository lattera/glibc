/* Copyright (C) 1991,1993,1995-1997,1999-2003,2004,2006,2009
	Free Software Foundation, Inc.
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
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <regex.h>


#define NEED_CHECK_SPEC \
  (!defined _XBS5_ILP32_OFF32 || !defined _XBS5_ILP32_OFFBIG \
   || !defined _XBS5_LP64_OFF64 || !defined _XBS5_LPBIG_OFFBIG \
   || !defined _POSIX_V6_ILP32_OFF32 || !defined _POSIX_V6_ILP32_OFFBIG \
   || !defined _POSIX_V6_LP64_OFF64 || !defined _POSIX_V6_LPBIG_OFFBIG \
   || !defined _POSIX_V7_ILP32_OFF32 || !defined _POSIX_V7_ILP32_OFFBIG \
   || !defined _POSIX_V7_LP64_OFF64 || !defined _POSIX_V7_LPBIG_OFFBIG)
#if NEED_CHECK_SPEC
static long int __sysconf_check_spec (const char *spec);
#endif


/* Get the value of the system variable NAME.  */
long int
__sysconf (name)
     int name;
{
  switch (name)
    {
      /* Also add obsolete or unnecessarily added constants here.  */
    case _SC_EQUIV_CLASS_MAX:
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
      return __get_child_max ();
#endif

    case _SC_CLK_TCK:
      return __getclktck ();

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
      return MAX (__tzname_max (), _POSIX_TZNAME_MAX);

    case _SC_JOB_CONTROL:
#if _POSIX_JOB_CONTROL > 0
      return _POSIX_JOB_CONTROL;
#else
      return -1;
#endif

    case _SC_SAVED_IDS:
#if _POSIX_SAVED_IDS > 0
      return 1;
#else
      return -1;
#endif

    case _SC_REALTIME_SIGNALS:
#if _POSIX_REALTIME_SIGNALS > 0
      return _POSIX_REALTIME_SIGNALS;
#else
      return -1;
#endif

    case _SC_PRIORITY_SCHEDULING:
#if _POSIX_PRIORITY_SCHEDULING > 0
      return _POSIX_PRIORITY_SCHEDULING;
#else
      return -1;
#endif

    case _SC_TIMERS:
#if _POSIX_TIMERS > 0
      return _POSIX_TIMERS;
#else
      return -1;
#endif

    case _SC_ASYNCHRONOUS_IO:
#if _POSIX_ASYNCHRONOUS_IO > 0
      return _POSIX_ASYNCHRONOUS_IO;
#else
      return -1;
#endif

    case _SC_PRIORITIZED_IO:
#if _POSIX_PRIORITIZED_IO > 0
      return _POSIX_PRIORITIZED_IO;
#else
      return -1;
#endif

    case _SC_SYNCHRONIZED_IO:
#if _POSIX_SYNCHRONIZED_IO > 0
      return _POSIX_SYNCHRONIZED_IO;
#else
      return -1;
#endif

    case _SC_FSYNC:
#if _POSIX_FSYNC > 0
      return _POSIX_FSYNC;
#else
      return -1;
#endif

    case _SC_MAPPED_FILES:
#if _POSIX_MAPPED_FILES > 0
      return _POSIX_MAPPED_FILES;
#else
      return -1;
#endif

    case _SC_MEMLOCK:
#if _POSIX_MEMLOCK > 0
      return _POSIX_MEMLOCK;
#else
      return -1;
#endif

    case _SC_MEMLOCK_RANGE:
#if _POSIX_MEMLOCK_RANGE > 0
      return _POSIX_MEMLOCK_RANGE;
#else
      return -1;
#endif

    case _SC_MEMORY_PROTECTION:
#if _POSIX_MEMORY_PROTECTION > 0
      return _POSIX_MEMORY_PROTECTION;
#else
      return -1;
#endif

    case _SC_MESSAGE_PASSING:
#if _POSIX_MESSAGE_PASSING > 0
      return _POSIX_MESSAGE_PASSING;
#else
      return -1;
#endif

    case _SC_SEMAPHORES:
#if _POSIX_SEMAPHORES > 0
      return _POSIX_SEMAPHORES;
#else
      return -1;
#endif

    case _SC_SHARED_MEMORY_OBJECTS:
#if _POSIX_SHARED_MEMORY_OBJECTS > 0
      return _POSIX_SHARED_MEMORY_OBJECTS;
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
#if _POSIX_PII > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_XTI:
#if _POSIX_PII_XTI > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_SOCKET:
#if _POSIX_PII_SOCKET > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_INTERNET:
#if _POSIX_PII_INTERNET > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI:
#if _POSIX_PII_OSI > 0
      return 1;
#else
      return -1;
#endif

    case _SC_POLL:
#if _POSIX_POLL > 0
      return 1;
#else
      return -1;
#endif

    case _SC_SELECT:
#if _POSIX_SELECT > 0
      return 1;
#else
      return -1;
#endif

      /* The same as _SC_IOV_MAX.  */
    case _SC_UIO_MAXIOV:
#ifdef	UIO_MAXIOV
      return UIO_MAXIOV;
#else
      return -1;
#endif

    case _SC_PII_INTERNET_STREAM:
#if _POSIX_PII_INTERNET_STREAM > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_INTERNET_DGRAM:
#if _POSIX_PII_INTERNET_DGRAM > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_COTS:
#if _POSIX_PII_OSI_COTS > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_CLTS:
#if _POSIX_PII_OSI_CLTS > 0
      return 1;
#else
      return -1;
#endif

    case _SC_PII_OSI_M:
#if _POSIX_PII_OSI_M > 0
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
      return _POSIX2_VERSION;

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

    case _SC_2_FORT_RUN:
#ifdef	_POSIX2_FORT_RUN
      return _POSIX2_FORT_RUN;
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
#if _POSIX_THREADS > 0
      return _POSIX_THREADS;
#else
      return -1;
#endif

    case _SC_THREAD_SAFE_FUNCTIONS:
#if _POSIX_THREAD_SAFE_FUNCTIONS > 0
      return _POSIX_THREAD_SAFE_FUNCTIONS;
#else
      return -1;
#endif

    case _SC_GETGR_R_SIZE_MAX:
      return NSS_BUFLEN_GROUP;

    case _SC_GETPW_R_SIZE_MAX:
      return NSS_BUFLEN_PASSWD;

    case _SC_LOGIN_NAME_MAX:
#ifdef	LOGIN_NAME_MAX
      return LOGIN_NAME_MAX;
#else
      return -1;
#endif

    case _SC_TTY_NAME_MAX:
#ifdef	TTY_NAME_MAX
      return TTY_NAME_MAX;
#else
      return -1;
#endif

    case _SC_THREAD_DESTRUCTOR_ITERATIONS:
#if _POSIX_THREAD_DESTRUCTOR_ITERATIONS > 0
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
#if _POSIX_THREAD_ATTR_STACKADDR > 0
      return _POSIX_THREAD_ATTR_STACKADDR;
#else
      return -1;
#endif

    case _SC_THREAD_ATTR_STACKSIZE:
#if _POSIX_THREAD_ATTR_STACKSIZE > 0
      return _POSIX_THREAD_ATTR_STACKSIZE;
#else
      return -1;
#endif

    case _SC_THREAD_PRIORITY_SCHEDULING:
#if _POSIX_THREAD_PRIORITY_SCHEDULING > 0
      return _POSIX_THREAD_PRIORITY_SCHEDULING;
#else
      return -1;
#endif

    case _SC_THREAD_PRIO_INHERIT:
#if _POSIX_THREAD_PRIO_INHERIT > 0
      return _POSIX_THREAD_PRIO_INHERIT;
#else
      return -1;
#endif

    case _SC_THREAD_PRIO_PROTECT:
#if _POSIX_THREAD_PRIO_PROTECT > 0
      return _POSIX_THREAD_PRIO_PROTECT;
#else
      return -1;
#endif

    case _SC_THREAD_PROCESS_SHARED:
#if _POSIX_THREAD_PROCESS_SHARED > 0
      return _POSIX_THREAD_PROCESS_SHARED;
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

    case _SC_XBS5_ILP32_OFF32:
#ifdef _XBS5_ILP32_OFF32
      return _XBS5_ILP32_OFF32;
#else
      return __sysconf_check_spec ("ILP32_OFF32");
#endif
    case _SC_XBS5_ILP32_OFFBIG:
#ifdef _XBS5_ILP32_OFFBIG
      return _XBS5_ILP32_OFFBIG;
#else
      return __sysconf_check_spec ("ILP32_OFFBIG");
#endif
    case _SC_XBS5_LP64_OFF64:
#ifdef _XBS5_LP64_OFF64
      return _XBS5_LP64_OFF64;
#else
      return __sysconf_check_spec ("LP64_OFF64");
#endif
    case _SC_XBS5_LPBIG_OFFBIG:
#ifdef _XBS5_LPBIG_OFFBIG
      return _XBS5_LPBIG_OFFBIG;
#else
      return __sysconf_check_spec ("LPBIG_OFFBIG");
#endif

    case _SC_V6_ILP32_OFF32:
#ifdef _POSIX_V6_ILP32_OFF32
      return _POSIX_V6_ILP32_OFF32;
#else
      return __sysconf_check_spec ("ILP32_OFF32");
#endif
    case _SC_V6_ILP32_OFFBIG:
#ifdef _POSIX_V6_ILP32_OFFBIG
      return _POSIX_V6_ILP32_OFFBIG;
#else
      return __sysconf_check_spec ("ILP32_OFFBIG");
#endif
    case _SC_V6_LP64_OFF64:
#ifdef _POSIX_V6_LP64_OFF64
      return _POSIX_V6_LP64_OFF64;
#else
      return __sysconf_check_spec ("LP64_OFF64");
#endif
    case _SC_V6_LPBIG_OFFBIG:
#ifdef _POSIX_V6_LPBIG_OFFBIG
      return _POSIX_V6_LPBIG_OFFBIG;
#else
      return __sysconf_check_spec ("LPBIG_OFFBIG");
#endif

    case _SC_V7_ILP32_OFF32:
#ifdef _POSIX_V7_ILP32_OFF32
      return _POSIX_V7_ILP32_OFF32;
#else
      return __sysconf_check_spec ("ILP32_OFF32");
#endif
    case _SC_V7_ILP32_OFFBIG:
#ifdef _POSIX_V7_ILP32_OFFBIG
      return _POSIX_V7_ILP32_OFFBIG;
#else
      return __sysconf_check_spec ("ILP32_OFFBIG");
#endif
    case _SC_V7_LP64_OFF64:
#ifdef _POSIX_V7_LP64_OFF64
      return _POSIX_V7_LP64_OFF64;
#else
      return __sysconf_check_spec ("LP64_OFF64");
#endif
    case _SC_V7_LPBIG_OFFBIG:
#ifdef _POSIX_V7_LPBIG_OFFBIG
      return _POSIX_V7_LPBIG_OFFBIG;
#else
      return __sysconf_check_spec ("LPBIG_OFFBIG");
#endif

    case _SC_XOPEN_LEGACY:
      return _XOPEN_LEGACY;

    case _SC_XOPEN_REALTIME:
#ifdef _XOPEN_REALTIME
      return _XOPEN_REALTIME;
#else
      return -1;
#endif
    case _SC_XOPEN_REALTIME_THREADS:
#ifdef _XOPEN_REALTIME_THREADS
      return _XOPEN_REALTIME_THREADS;
#else
      return -1;
#endif

    case _SC_ADVISORY_INFO:
#if _POSIX_ADVISORY_INFO > 0
      return _POSIX_ADVISORY_INFO;
#else
      return -1;
#endif

    case _SC_BARRIERS:
#if _POSIX_BARRIERS > 0
      return _POSIX_BARRIERS;
#else
      return -1;
#endif

    case _SC_BASE:
#if _POSIX_BASE > 0
      return _POSIX_BASE;
#else
      return -1;
#endif
    case _SC_C_LANG_SUPPORT:
#if _POSIX_C_LANG_SUPPORT > 0
      return _POSIX_C_LANG_SUPPORT;
#else
      return -1;
#endif
    case _SC_C_LANG_SUPPORT_R:
#if _POSIX_C_LANG_SUPPORT_R > 0
      return _POSIX_C_LANG_SUPPORT_R;
#else
      return -1;
#endif

    case _SC_CLOCK_SELECTION:
#if _POSIX_CLOCK_SELECTION > 0
      return _POSIX_CLOCK_SELECTION;
#else
      return -1;
#endif

    case _SC_CPUTIME:
#if _POSIX_CPUTIME > 0
      return _POSIX_CPUTIME;
#else
      return -1;
#endif

    case _SC_DEVICE_IO:
#if _POSIX_DEVICE_IO > 0
      return _POSIX_DEVICE_IO;
#else
      return -1;
#endif
    case _SC_DEVICE_SPECIFIC:
#if _POSIX_DEVICE_SPCIFIC > 0
      return _POSIX_DEVICE_SPECIFIC;
#else
      return -1;
#endif
    case _SC_DEVICE_SPECIFIC_R:
#if _POSIX_DEVICE_SPCIFIC_R > 0
      return _POSIX_DEVICE_SPECIFIC_R;
#else
      return -1;
#endif

    case _SC_FD_MGMT:
#if _POSIX_FD_MGMT > 0
      return _POSIX_FD_MGMT;
#else
      return -1;
#endif

    case _SC_FIFO:
#if _POSIX_FIFO > 0
      return _POSIX_FIFO;
#else
      return -1;
#endif
    case _SC_PIPE:
#if _POSIX_PIPE > 0
      return _POSIX_PIPE;
#else
      return -1;
#endif

    case _SC_FILE_ATTRIBUTES:
#if _POSIX_FILE_ATTRIBUTES > 0
      return _POSIX_FILE_ATTRIBUTES;
#else
      return -1;
#endif
    case _SC_FILE_LOCKING:
#if _POSIX_FILE_LOCKING > 0
      return _POSIX_FILE_LOCKING;
#else
      return -1;
#endif
    case _SC_FILE_SYSTEM:
#if _POSIX_FILE_SYSTEM > 0
      return _POSIX_FILE_SYSTEM;
#else
      return -1;
#endif

    case _SC_MONOTONIC_CLOCK:
#if _POSIX_MONOTONIC_CLOCK
      return _POSIX_MONOTONIC_CLOCK;
#else
      return -1;
#endif

    case _SC_MULTI_PROCESS:
#if _POSIX_MULTI_PROCESS > 0
      return _POSIX_MULTI_PROCESS;
#else
      return -1;
#endif
    case _SC_SINGLE_PROCESS:
#if _POSIX_SINGLE_PROCESS > 0
      return _POSIX_SINGLE_PROCESS;
#else
      return -1;
#endif

    case _SC_NETWORKING:
#if _POSIX_NETWORKING > 0
      return _POSIX_NETWORKING;
#else
      return -1;
#endif

    case _SC_READER_WRITER_LOCKS:
#if _POSIX_READER_WRITER_LOCKS > 0
      return _POSIX_READER_WRITER_LOCKS;
#else
      return -1;
#endif
    case _SC_SPIN_LOCKS:
#if _POSIX_SPIN_LOCKS > 0
      return _POSIX_SPIN_LOCKS;
#else
      return -1;
#endif

    case _SC_REGEXP:
#if _POSIX_REGEXP > 0
      return _POSIX_REGEXP;
#else
      return -1;
#endif
    case _SC_REGEX_VERSION:
#if _POSIX_REGEX_VERSION > 0
      return _POSIX_REGEX_VERSION;
#else
      return -1;
#endif

    case _SC_SHELL:
#if _POSIX_SHELL > 0
      return _POSIX_SHELL;
#else
      return -1;
#endif

    case _SC_SIGNALS:
#ifdef _POSUX_SIGNALS
      return _POSIX_SIGNALS;
#else
      return -1;
#endif

    case _SC_SPAWN:
#if _POSIX_SPAWN > 0
      return _POSIX_SPAWN;
#else
      return -1;
#endif

    case _SC_SPORADIC_SERVER:
#if _POSIX_SPORADIC_SERVER > 0
      return _POSIX_SPORADIC_SERVER;
#else
      return -1;
#endif
    case _SC_THREAD_SPORADIC_SERVER:
#if _POSIX_THREAD_SPORADIC_SERVER > 0
      return _POSIX_THREAD_SPORADIC_SERVER;
#else
      return -1;
#endif

    case _SC_SYSTEM_DATABASE:
#if _POSIX_SYSTEM_DATABASE > 0
      return _POSIX_SYSTEM_DATABASE;
#else
      return -1;
#endif
    case _SC_SYSTEM_DATABASE_R:
#if _POSIX_SYSTEM_DATABASE_R > 0
      return _POSIX_SYSTEM_DATABASE_R;
#else
      return -1;
#endif

    case _SC_THREAD_CPUTIME:
#if _POSIX_THREAD_CPUTIME > 0
      return _POSIX_THREAD_CPUTIME;
#else
      return -1;
#endif

    case _SC_TIMEOUTS:
#if _POSIX_TIMEOUTS > 0
      return _POSIX_TIMEOUTS;
#else
      return -1;
#endif

    case _SC_TYPED_MEMORY_OBJECTS:
#if _POSIX_TYPED_MEMORY_OBJECTS > 0
      return _POSIX_TYPED_MEMORY_OBJECTS;
#else
      return -1;
#endif

    case _SC_USER_GROUPS:
#if _POSIX_USER_GROUPS > 0
      return _POSIX_USER_GROUPS;
#else
      return -1;
#endif
    case _SC_USER_GROUPS_R:
#if _POSIX_USER_GROUPS_R > 0
      return _POSIX_USER_GROUPS_R;
#else
      return -1;
#endif

    case _SC_2_PBS:
#ifdef _POSIX2_PBS
      return _POSIX2_PBS;
#else
      return -1;
#endif
    case _SC_2_PBS_ACCOUNTING:
#ifdef _POSIX2_PBS_ACCOUNTING
      return _POSIX2_PBS_ACCOUNTING;
#else
      return -1;
#endif
    case _SC_2_PBS_CHECKPOINT:
#ifdef _POSIX2_PBS_CHECKPOINT
      return _POSIX2_PBS_CHECKPOINT;
#else
      return -1;
#endif
    case _SC_2_PBS_LOCATE:
#ifdef _POSIX2_PBS_LOCATE
      return _POSIX2_PBS_LOCATE;
#else
      return -1;
#endif
    case _SC_2_PBS_MESSAGE:
#ifdef _POSIX2_PBS_MESSAGE
      return _POSIX2_PBS_MESSAGE;
#else
      return -1;
#endif
    case _SC_2_PBS_TRACK:
#ifdef _POSIX2_PBS_TRACK
      return _POSIX2_PBS_TRACK;
#else
      return -1;
#endif

    case _SC_SYMLOOP_MAX:
#ifdef SYMLOOP_MAX
      return SYMLOOP_MAX;
#else
      return -1;
#endif

    case _SC_STREAMS:
#ifdef _XOPEN_STREAMS
      return _XOPEN_STREAMS;
#else
      return -1;
#endif

    case _SC_HOST_NAME_MAX:
#ifdef HOST_NAME_MAX
      return HOST_NAME_MAX;
#else
      return -1;
#endif

    case _SC_TRACE:
#if _POSIX_TRACE > 0
      return _POSIX_TRACE;
#else
      return -1;
#endif
    case _SC_TRACE_EVENT_FILTER:
#if _POSIX_TRACE_EVENT_FILTER > 0
      return _POSIX_TRACE_EVENT_FILTER;
#else
      return -1;
#endif
    case _SC_TRACE_INHERIT:
#if _POSIX_TRACE_INHERIT > 0
      return _POSIX_TRACE_INHERIT;
#else
      return -1;
#endif
    case _SC_TRACE_LOG:
#if _POSIX_TRACE_LOG > 0
      return _POSIX_TRACE_LOG;
#else
      return -1;
#endif

    case _SC_TRACE_EVENT_NAME_MAX:
    case _SC_TRACE_NAME_MAX:
    case _SC_TRACE_SYS_MAX:
    case _SC_TRACE_USER_EVENT_MAX:
      /* No support for tracing.  */

    case _SC_XOPEN_STREAMS:
      /* No support for STREAMS.  */
      return -1;

    case _SC_LEVEL1_ICACHE_SIZE:
    case _SC_LEVEL1_ICACHE_ASSOC:
    case _SC_LEVEL1_ICACHE_LINESIZE:
    case _SC_LEVEL1_DCACHE_SIZE:
    case _SC_LEVEL1_DCACHE_ASSOC:
    case _SC_LEVEL1_DCACHE_LINESIZE:
    case _SC_LEVEL2_CACHE_SIZE:
    case _SC_LEVEL2_CACHE_ASSOC:
    case _SC_LEVEL2_CACHE_LINESIZE:
    case _SC_LEVEL3_CACHE_SIZE:
    case _SC_LEVEL3_CACHE_ASSOC:
    case _SC_LEVEL3_CACHE_LINESIZE:
    case _SC_LEVEL4_CACHE_SIZE:
    case _SC_LEVEL4_CACHE_ASSOC:
      /* In general we cannot determine these values.  Therefore we
	 return zero which indicates that no information is
	 available.  */
      return 0;

    case _SC_IPV6:
#if _POSIX_IPV6 > 0
      return _POSIX_IPV6;
#else
      return -1;
#endif

    case _SC_RAW_SOCKETS:
#if _POSIX_RAW_SOCKETS > 0
      return _POSIX_RAW_SOCKETS;
#else
      return -1;
#endif
    }
}

#undef __sysconf
weak_alias (__sysconf, sysconf)
libc_hidden_def (__sysconf)

#if NEED_CHECK_SPEC
static long int
__sysconf_check_spec (const char *spec)
{
  int save_errno = errno;

  const char *getconf_dir = __secure_getenv ("GETCONF_DIR") ?: GETCONF_DIR;
  size_t getconf_dirlen = strlen (getconf_dir);
  size_t speclen = strlen (spec);

  char name[getconf_dirlen + sizeof ("/POSIX_V6_") + speclen];
  memcpy (mempcpy (mempcpy (name, getconf_dir, getconf_dirlen),
		   "/POSIX_V6_", sizeof ("/POSIX_V6_") - 1),
	  spec, speclen + 1);

  struct stat64 st;
  long int ret = __xstat64 (_STAT_VER, name, &st) >= 0 ? 1 : -1;

  __set_errno (save_errno);
  return ret;
}
#endif
