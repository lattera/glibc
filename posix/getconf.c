/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <libintl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct conf
  {
    const char *name;
    const int call_name;
    const enum { SYSCONF, CONFSTR, PATHCONF } call;
  };

static struct conf vars[] =
  {
    { "LINK_MAX", _PC_LINK_MAX, PATHCONF },
    { "MAX_CANON", _PC_MAX_CANON, PATHCONF },
    { "MAX_INPUT", _PC_MAX_INPUT, PATHCONF },
    { "NAME_MAX", _PC_NAME_MAX, PATHCONF },
    { "PATH_MAX", _PC_PATH_MAX, PATHCONF },
    { "PIPE_BUF", _PC_PIPE_BUF, PATHCONF },
    { "SOCK_MAXBUF", _PC_SOCK_MAXBUF, PATHCONF },
    { "_POSIX_CHOWN_RESTRICTED", _PC_CHOWN_RESTRICTED, PATHCONF },
    { "_POSIX_NO_TRUNC", _PC_NO_TRUNC, PATHCONF },
    { "_POSIX_VDISABLE", _PC_VDISABLE, PATHCONF },
    { "_POSIX_SYNC_IO", _PC_SYNC_IO, PATHCONF },
    { "_POSIX_ASYNC_IO", _PC_ASYNC_IO, PATHCONF },
    { "_POSIX_PRIO_IO", _PC_PRIO_IO, PATHCONF },

    { "ARG_MAX", _SC_ARG_MAX, SYSCONF },
    { "CHILD_MAX", _SC_CHILD_MAX, SYSCONF },
    { "CLK_TCK", _SC_CLK_TCK, SYSCONF },
    { "NGROUPS_MAX", _SC_NGROUPS_MAX, SYSCONF },
    { "OPEN_MAX", _SC_OPEN_MAX, SYSCONF },
    { "_POSIX_JOB_CONTROL", _SC_JOB_CONTROL, SYSCONF },
    { "_POSIX_SAVED_IDS", _SC_SAVED_IDS, SYSCONF },
    { "_POSIX_REALTIME_SIGNALS", _SC_REALTIME_SIGNALS, SYSCONF },
    { "_POSIX_PRIORITY_SCHEDULING", _SC_PRIORITY_SCHEDULING, SYSCONF },
    { "_POSIX_TIMERS", _SC_TIMERS, SYSCONF },
    { "_POSIX_ASYNCHRONOUS_IO", _SC_ASYNCHRONOUS_IO, SYSCONF },
    { "_POSIX_PRIORITIZED_IO", _SC_PRIORITIZED_IO, SYSCONF },
    { "_POSIX_SYNCHRONIZED_IO", _SC_SYNCHRONIZED_IO, SYSCONF },
    { "_POSIX_FSYNC", _SC_FSYNC, SYSCONF },
    { "_POSIX_MAPPED_FILES", _SC_MAPPED_FILES, SYSCONF },
    { "_POSIX_MEMLOCK", _SC_MEMLOCK, SYSCONF },
    { "_POSIX_MEMLOCK_RANGE", _SC_MEMLOCK_RANGE, SYSCONF },
    { "_POSIX_MEMORY_PROTECTION", _SC_MEMORY_PROTECTION, SYSCONF },
    { "_POSIX_MESSAGE_PASSING", _SC_MESSAGE_PASSING, SYSCONF },
    { "_POSIX_SEMAPHORES", _SC_SEMAPHORES, SYSCONF },
    { "_POSIX_SHARED_MEMORY_OBJECTS", _SC_SHARED_MEMORY_OBJECTS, SYSCONF },
    { "_POSIX_VERSION", _SC_VERSION, SYSCONF },
    { "_POSIX_PII", _SC_PII, SYSCONF },
    { "_POSIX_PII_XTI", _SC_PII_XTI, SYSCONF },
    { "_POSIX_PII_SOCKET", _SC_PII_SOCKET, SYSCONF },
    { "_POSIX_PII_INTERNET", _SC_PII_INTERNET, SYSCONF },
    { "_POSIX_PII_OSI", _SC_PII_OSI, SYSCONF },
    { "_POSIX_POLL", _SC_POLL, SYSCONF },
    { "_POSIX_SELECT", _SC_SELECT, SYSCONF },
    { "UIO_MAXIOV", _SC_UIO_MAXIOV, SYSCONF },
    { "_POSIX_PII_INTERNET_STREAM", _SC_PII_INTERNET_STREAM, SYSCONF },
    { "_POSIX_PII_INTERNET_DGRAM", _SC_PII_INTERNET_DGRAM, SYSCONF },
    { "_POSIX_PII_OSI_COTS", _SC_PII_OSI_COTS, SYSCONF },
    { "_POSIX_PII_OSI_CLTS", _SC_PII_OSI_CLTS, SYSCONF },
    { "_POSIX_PII_OSI_M", _SC_PII_OSI_M, SYSCONF },
    { "_T_IOV_MAX", _SC_T_IOV_MAX, SYSCONF },
    { "_POSIX_THREADS", _SC_THREADS, SYSCONF },
    { "_POSIX_THREAD_SAFE_FUNCTIONS", _SC_THREAD_SAFE_FUNCTIONS, SYSCONF },
    { "NSS_BUFLEN_GROUP", _SC_GETGR_R_SIZE_MAX, SYSCONF },
    { "NSS_BUFLEN_PASSWD", _SC_GETPW_R_SIZE_MAX, SYSCONF },
    { "LOGNAME_MAX", _SC_LOGIN_NAME_MAX, SYSCONF },
    { "TTY_NAME_MAX", _SC_TTY_NAME_MAX, SYSCONF },
    { "PTHREAD_DESTRUCTOR_ITERATIONS", _SC_THREAD_DESTRUCTOR_ITERATIONS, SYSCONF },
    { "PTHREAD_KEYS_MAX", _SC_THREAD_KEYS_MAX, SYSCONF },
    { "PTHREAD_STACK_MIN", _SC_THREAD_STACK_MIN, SYSCONF },
    { "PTHREAD_THREADS_MAX", _SC_THREAD_THREADS_MAX, SYSCONF },
    { "_POSIX_THREAD_ATTR_STACKADDR", _SC_THREAD_ATTR_STACKADDR, SYSCONF },
    { "_POSIX_THREAD_ATTR_STACKSIZE", _SC_THREAD_ATTR_STACKSIZE, SYSCONF },
    { "_POSIX_THREAD_PRIORITY_SCHEDULING", _SC_THREAD_PRIORITY_SCHEDULING, SYSCONF },
    { "_POSIX_THREAD_PRIO_INHERIT", _SC_THREAD_PRIO_INHERIT, SYSCONF },
    { "_POSIX_THREAD_PRIO_PROTECT", _SC_THREAD_PRIO_PROTECT, SYSCONF },
    { "_POSIX_THREAD_PROCESS_SHARED", _SC_THREAD_PROCESS_SHARED, SYSCONF },
    /* POSIX.2  */
    { "BC_BASE_MAX", _SC_BC_BASE_MAX, SYSCONF },
    { "BC_DIM_MAX", _SC_BC_DIM_MAX, SYSCONF },
    { "BC_SCALE_MAX", _SC_BC_SCALE_MAX, SYSCONF },
    { "BC_STRING_MAX", _SC_BC_STRING_MAX, SYSCONF },
    { "COLL_WEIGHTS_MAX", _SC_COLL_WEIGHTS_MAX, SYSCONF },
    { "EQUIV_CLASS_MAX", _SC_EQUIV_CLASS_MAX, SYSCONF },
    { "EXPR_NEST_MAX", _SC_EXPR_NEST_MAX, SYSCONF },
    { "LINE_MAX", _SC_LINE_MAX, SYSCONF },
    { "RE_DUP_MAX", _SC_RE_DUP_MAX, SYSCONF },
    { "CHARCLASS_NAME_MAX", _SC_CHARCLASS_NAME_MAX },
    { "POSIX2_C_BIND", _SC_2_C_BIND, SYSCONF },
    { "POSIX2_C_DEV", _SC_2_C_DEV, SYSCONF },
    { "POSIX2_FORT_DEV", _SC_2_FORT_DEV, SYSCONF },
    { "POSIX2_FORT_RUN", _SC_2_FORT_RUN, SYSCONF },
    { "POSIX2_LOCALEDEF", _SC_2_LOCALEDEF, SYSCONF },
    { "POSIX2_SW_DEV", _SC_2_SW_DEV, SYSCONF },

    { "PATH", _CS_PATH, CONFSTR },

    { NULL, 0, SYSCONF }
  };

extern const char *__progname;


static void
usage (void)
{
  fprintf (stderr, _("Usage: %s variable_name [pathname]\n"), __progname);
  exit (2);
}

int
main (int argc, char *argv[])
{
  register const struct conf *c;

  if (argc < 2 || argc > 3)
    usage ();

  for (c = vars; c->name != NULL; ++c)
    if (!strcmp (c->name, argv[1]))
      {
	long int value;
	size_t clen;
	char *cvalue;
	switch (c->call)
	  {
	  case PATHCONF:
	    if (argc < 3)
	      usage ();
	    value = pathconf (argv[2], c->call_name);
	    if (value == -1)
	      error (3, errno, "pathconf: %s", argv[2]);

	    printf ("%ld\n", value);
	    exit (0);

	  case SYSCONF:
	    if (argc > 2)
	      usage ();
	    value = sysconf (c->call_name);
	    printf ("%ld\n", value);
	    exit (0);

	  case CONFSTR:
	    if (argc > 2)
	      usage ();
	    clen = confstr (c->call_name, (char *) NULL, 0);
	    cvalue = (char *) malloc (clen);
	    if (cvalue == NULL)
	      error (3, 0, _("memory exhausted"));

	    if (confstr (c->call_name, cvalue, clen) != clen)
	      error (3, errno, "confstr");

	    printf ("%.*s\n", (int) clen, cvalue);
	    exit (0);
	  }
      }

  error (2, 0, _("Unrecognized variable `%s'"), argv[1]);
  /* NOTREACHED */
  return 2;
}
