/* `sysconf', `pathconf', and `confstr' NAME values.  Generic version.
Copyright (C) 1993, 1995, 1996 Free Software Foundation, Inc.
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

/* Values for the NAME argument to `pathconf' and `fpathconf'.  */
enum
  {
    _PC_LINK_MAX,
#define	_PC_LINK_MAX		_PC_LINK_MAX
    _PC_MAX_CANON,
#define	_PC_MAX_CANON		_PC_MAX_CANON
    _PC_MAX_INPUT,
#define	_PC_MAX_INPUT		_PC_MAX_INPUT
    _PC_NAME_MAX,
#define	_PC_NAME_MAX		_PC_NAME_MAX
    _PC_PATH_MAX,
#define	_PC_PATH_MAX		_PC_PATH_MAX
    _PC_PIPE_BUF,
#define	_PC_PIPE_BUF		_PC_PIPE_BUF
    _PC_CHOWN_RESTRICTED,
#define	_PC_CHOWN_RESTRICTED	_PC_CHOWN_RESTRICTED
    _PC_NO_TRUNC,
#define	_PC_NO_TRUNC		_PC_NO_TRUNC
    _PC_VDISABLE,
    _PC_SYNC_IO,
#define	_PC_SYNC_IO		_PC_SYNC_IO
    _PC_ASYNC_IO,
#define	_PC_ASYNC_IO		_PC_ASYNC_IO
    _PC_PRIO_IO
#define	_PC_PRIO_IO		_PC_PRIO_IO
  };

/* Values for the argument to `sysconf'.  */
enum
  {
    _SC_ARG_MAX,
#define	_SC_ARG_MAX		_SC_ARG_MAX
    _SC_CHILD_MAX,
#define	_SC_CHILD_MAX		_SC_CHILD_MAX
    _SC_CLK_TCK,
#define	_SC_CLK_TCK		_SC_CLK_TCK
    _SC_NGROUPS_MAX,
#define	_SC_NGROUPS_MAX		_SC_NGROUPS_MAX
    _SC_OPEN_MAX,
#define	_SC_OPEN_MAX		_SC_OPEN_MAX
    _SC_STREAM_MAX,
#define	_SC_STREAM_MAX		_SC_STREAM_MAX
    _SC_TZNAME_MAX,
#define	_SC_TZNAME_MAX		_SC_TZNAME_MAX
    _SC_JOB_CONTROL,
#define	_SC_JOB_CONTROL		_SC_JOB_CONTROL
    _SC_SAVED_IDS,
#define	_SC_SAVED_IDS		_SC_SAVED_IDS
    _SC_REALTIME_SIGNALS,
#define	_SC_REALTIME_SIGNALS	_SC_REALTIME_SIGNALS
    _SC_PRIORITY_SCHEDULING,
#define	_SC_PRIORITY_SCHEDULING	_SC_PRIORITY_SCHEDULING
    _SC_TIMERS,
#define	_SC_TIMERS		_SC_TIMERS
    _SC_ASYNCHRONOUS_IO,
#define	_SC_ASYNCHRONOUS_IO	_SC_ASYNCHRONOUS_IO
    _SC_PRIORITIZED_IO,
#define	_SC_PRIORITIZED_IO	_SC_PRIORITIZED_IO
    _SC_SYNCHRONIZED_IO,
#define	_SC_SYNCHRONIZED_IO	_SC_SYNCHRONIZED_IO
    _SC_FSYNC,
#define	_SC_FSYNC		_SC_FSYNC
    _SC_MAPPED_FILES,
#define	_SC_MAPPED_FILES	_SC_MAPPED_FILES
    _SC_MEMLOCK,
#define	_SC_MEMLOCK		_SC_MEMLOCK
    _SC_MEMLOCK_RANGE,
#define	_SC_MEMLOCK_RANGE	_SC_MEMLOCK_RANGE
    _SC_MEMORY_PROTECTION,
#define	_SC_MEMORY_PROTECTION	_SC_MEMORY_PROTECTION
    _SC_MESSAGE_PASSING,
#define	_SC_MESSAGE_PASSING	_SC_MESSAGE_PASSING
    _SC_SEMAPHORES,
#define	_SC_SEMAPHORES		_SC_SEMAPHORES
    _SC_SHARED_MEMORY_OBJECTS,
#define	_SC_SHARED_MEMORY_OBJECTS	_SC_SHARED_MEMORY_OBJECTS
    _SC_AIO_LISTIO_MAX,
#define	_SC_AIO_LIST_MAX	_SC_AIO_LIST_MAX
    _SC_AIO_MAX,
#define	_SC_AIO_MAX		_SC_AIO_MAX
    _SC_AIO_PRIO_DELTA_MAX,
#define	_SC_AIO_PRIO_DELTA_MAX	_SC_AIO_PRIO_DELTA_MAX
    _SC_DELAYTIMER_MAX,
#define	_SC_DELAYTIMER_MAX	_SC_DELAYTIMER_MAX
    _SC_MQ_OPEN_MAX,
#define	_SC_MQ_OPEN_MAX		_SC_MQ_OPEN_MAX
    _SC_MQ_PRIO_MAX,
#define	_SC_MQ_PRIO_MAX		_SC_MQ_PRIO_MAX
    _SC_VERSION,
#define	_SC_VERSION		_SC_VERSION
    _SC_PAGESIZE,
#define	_SC_PAGESIZE		_SC_PAGESIZE
    _SC_RTSIG_MAX,
#define	_SC_RTSIG_MAX		_SC_RTSIG_MAX
    _SC_SEM_NSEMS_MAX,
#define	_SC_SEM_NSEMS_MAX	_SC_SEM_NSEMS_MAX
    _SC_SEM_VALUE_MAX,
#define	_SC_SEM_VALUE_MAX	_SC_SEM_VALUE_MAX
    _SC_SIGQUEUE_MAX,
#define	_SC_SIGQUEUE_MAX	_SC_SIGQUEUE_MAX
    _SC_TIMER_MAX,
#define	_SC_TIMER_MAX		_SC_TIMER_MAX

    /* Values for the argument to `sysconf'
       corresponding to _POSIX2_* symbols.  */
    _SC_BC_BASE_MAX,
#define	_SC_BC_BASE_MAX		_SC_BC_BASE_MAX
    _SC_BC_DIM_MAX,
#define	_SC_BC_DIM_MAX		_SC_BC_DIM_MAX
    _SC_BC_SCALE_MAX,
#define	_SC_BC_SCALE_MAX	_SC_BC_SCALE_MAX
    _SC_BC_STRING_MAX,
#define	_SC_BC_STRING_MAX	_SC_BC_STRING_MAX
    _SC_COLL_WEIGHTS_MAX,
#define	_SC_COLL_WEIGHTS_MAX	_SC_COLL_WEIGHTS_MAX
    _SC_EQUIV_CLASS_MAX,
#define	_SC_EQUIV_CLASS_MAX	_SC_EQUIV_CLASS_MAX
    _SC_EXPR_NEST_MAX,
#define	_SC_EXPR_NEST_MAX	_SC_EXPR_NEST_MAX
    _SC_LINE_MAX,
#define	_SC_LINE_MAX		_SC_LINE_MAX
    _SC_RE_DUP_MAX,
#define	_SC_RE_DUP_MAX		_SC_RE_DUP_MAX
    _SC_CHARCLASS_NAME_MAX,
#define	_SC_CHARCLASS_NAME_MAX	_SC_CHARCLASS_NAME_MAX

    _SC_2_VERSION,
#define	_SC_2_VERSION		_SC_2_VERSION
    _SC_2_C_BIND,
#define	_SC_2_C_BIND		_SC_2_C_BIND
    _SC_2_C_DEV,
#define	_SC_2_C_DEV		_SC_2_C_DEV
    _SC_2_FORT_DEV,
#define	_SC_2_FORT_DEV		_SC_2_FORT_DEV
    _SC_2_FORT_RUN,
#define	_SC_2_FORT_RUN		_SC_2_FORT_RUN
    _SC_2_SW_DEV,
#define	_SC_2_SW_DEV		_SC_2_SW_DEV
    _SC_2_LOCALEDEF
  };

#ifdef __USE_POSIX2
/* Values for the NAME argument to `confstr'.  */
enum
  {
    _CS_PATH			/* The default search path.  */
  };
#endif
