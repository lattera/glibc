/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_sleep.c	10.6 (Sleepycat) 6/28/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <errno.h>
#ifndef HAVE_SYS_TIME_H
#include <time.h>
#endif
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_ext.h"

/*
 * __db_sleep --
 *	Yield the processor for a period of time.
 *
 * PUBLIC: int __db_sleep __P((u_long, u_long));
 */
int
__db_sleep(secs, usecs)
	u_long secs, usecs;		/* Seconds and microseconds. */
{
#ifndef _WIN32
	struct timeval t;
#endif

	/* Don't require that the values be normalized. */
	for (; usecs >= 1000000; ++secs, usecs -= 1000000);

	/*
	 * It's important that we yield the processor here so that other
	 * processes or threads are permitted to run.
	 */
#ifdef _WIN32
	Sleep(secs * 1000 + usecs / 1000);
	return (0);
#else
	t.tv_sec = secs;
	t.tv_usec = usecs;
	return (select(0, NULL, NULL, NULL, &t) == -1 ? errno : 0);
#endif
}
