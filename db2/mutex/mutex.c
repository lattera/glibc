/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mutex.c	10.25 (Sleepycat) 9/23/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "common_ext.h"

#ifdef HAVE_SPINLOCKS

#ifdef HAVE_FUNC_AIX
#define	TSL_INIT(x)
#define	TSL_SET(x)	(!_check_lock(x, 0, 1))
#define	TSL_UNSET(x)	_clear_lock(x, 0)
#endif

#ifdef HAVE_ASSEM_MC68020_GCC
#include "68020.gcc"
#endif

#if defined(HAVE_FUNC_MSEM)
/*
 * XXX
 * Should we not use MSEM_IF_NOWAIT and let the system block for us?
 * I've no idea if this will block all threads in the process or not.
 */
#define	TSL_INIT(x)	msem_init(x, MSEM_UNLOCKED)
#define	TSL_SET(x)	(!msem_lock(x, MSEM_IF_NOWAIT))
#define	TSL_UNSET(x)	msem_unlock(x, 0)
#endif

#ifdef HAVE_FUNC_SGI
#define	TSL_INIT(x)	init_lock(x)
#define	TSL_SET(x)	(!acquire_lock(x))
#define	TSL_UNSET(x)	release_lock(x)
#endif

#ifdef HAVE_FUNC_SOLARIS
/*
 * Semaphore calls don't work on Solaris 5.5.
 *
 * #define	TSL_INIT(x)	sema_init(x, 1, USYNC_PROCESS, NULL)
 * #define	TSL_SET(x)	(sema_wait(x) == 0)
 * #define	TSL_UNSET(x)	sema_post(x)
 */
#define	TSL_INIT(x)
#define	TSL_SET(x)	(_lock_try(x))
#define	TSL_UNSET(x)	_lock_clear(x)
#endif

#ifdef HAVE_ASSEM_SPARC_GCC
#include "sparc.gcc"
#endif

#ifdef HAVE_ASSEM_UTS4_CC
#define TSL_INIT(x)
#define TSL_SET(x)	(!uts_lock(x, 1))
#define TSL_UNSET(x)	(*(x) = 0)
#endif

#ifdef HAVE_ASSEM_X86_GCC
#include "x86.gcc"
#endif

#if defined(_WIN32)
/* DBDB this needs to be byte-aligned!! */
#define	TSL_INIT(tsl)
#define	TSL_SET(tsl)	(!InterlockedExchange((PLONG)tsl, 1))
#define	TSL_UNSET(tsl)	(*(tsl) = 0)
#endif

#ifdef macintosh
/* Mac spinlocks are simple because we cannot possibly be preempted. */
#define	TSL_INIT(tsl)
#define	TSL_SET(tsl)	(*(tsl) = 1)
#define	TSL_UNSET(tsl)	(*(tsl) = 0)
#endif

#endif /* HAVE_SPINLOCKS */

#ifdef	MORE_THAN_ONE_PROCESSOR
#define	TSL_DEFAULT_SPINS	5	/* Default spins before block. */
#else
#define	TSL_DEFAULT_SPINS	1	/* Default spins before block. */
#endif

/*
 * __db_mutex_init --
 *	Initialize a DB mutex structure.
 *
 * PUBLIC: void __db_mutex_init __P((db_mutex_t *, off_t));
 */
void
__db_mutex_init(mp, off)
	db_mutex_t *mp;
	off_t off;
{
#ifdef DEBUG
	if ((ALIGNTYPE)mp & (MUTEX_ALIGNMENT - 1)) {
		(void)fprintf(stderr,
		    "MUTEX ERROR: mutex NOT %d-byte aligned!\n",
		    MUTEX_ALIGNMENT);
		abort();
	}
#endif
	memset(mp, 0, sizeof(db_mutex_t));

#ifdef HAVE_SPINLOCKS
	TSL_INIT(&mp->tsl_resource);
#else
	mp->off = off;
#endif
}

#define	MS(n)		((n) * 1000)	/* Milliseconds to micro-seconds. */
#define	SECOND		(MS(1000))	/* A second's worth of micro-seconds. */

/*
 * __db_mutex_lock
 *	Lock on a mutex, logically blocking if necessary.
 *
 * PUBLIC: int __db_mutex_lock __P((db_mutex_t *, int, int (*)(void)));
 */
int
__db_mutex_lock(mp, fd, yield)
	db_mutex_t *mp;
	int fd;
	int (*yield) __P((void));
{
	u_long usecs;

#ifdef HAVE_SPINLOCKS
	int nspins;

	for (usecs = MS(10);;) {
		/*
		 * Try and acquire the uncontested resource lock for
		 * TSL_DEFAULT_SPINS.
		 */
		for (nspins = TSL_DEFAULT_SPINS; nspins > 0; --nspins)
			if (TSL_SET(&mp->tsl_resource)) {
#ifdef DEBUG
				if (mp->pid != 0) {
					(void)fprintf(stderr,
		    "MUTEX ERROR: __db_mutex_lock: lock currently locked\n");
					abort();
				}
				mp->pid = getpid();
#endif
#ifdef MUTEX_STATISTICS
				if (usecs == MS(10))
					++mp->mutex_set_nowait;
				else
					++mp->mutex_set_wait;
#endif
				return (0);
			}

		/* Yield the processor; wait 10ms initially, up to 1 second. */
		if (yield == NULL || yield() != 0) {
			(void)__db_sleep(0, usecs);
			if ((usecs <<= 1) > SECOND)
				usecs = SECOND;
		}
	}
	/* NOTREACHED */

#else /* !HAVE_SPINLOCKS */
	struct flock k_lock;
	pid_t mypid;
	int locked;

	/* Initialize the lock. */
	k_lock.l_whence = SEEK_SET;
	k_lock.l_start = mp->off;
	k_lock.l_len = 1;

	for (locked = 0, mypid = getpid();;) {
		/*
		 * Wait for the lock to become available; wait 10ms initially,
		 * up to 1 second.
		 */
		for (usecs = MS(10); mp->pid != 0;)
			if (yield == NULL || yield() != 0) {
				(void)__db_sleep(0, usecs);
				if ((usecs <<= 1) > SECOND)
					usecs = SECOND;
			}

		/* Acquire an exclusive kernel lock. */
		k_lock.l_type = F_WRLCK;
		if (fcntl(fd, F_SETLKW, &k_lock))
			return (1);

		/* If the resource tsl is still available, it's ours. */
		if (mp->pid == 0) {
			locked = 1;
			mp->pid = mypid;
		}

		/* Release the kernel lock. */
		k_lock.l_type = F_UNLCK;
		if (fcntl(fd, F_SETLK, &k_lock))
			return (1);

		/*
		 * If we got the resource tsl we're done.
		 *
		 * !!!
		 * We can't check to see if the lock is ours, because we may
		 * be trying to block ourselves in the lock manager, and so
		 * the holder of the lock that's preventing us from getting
		 * the lock may be us!  (Seriously.)
		 */
		if (locked)
			break;
	}

#ifdef MUTEX_STATISTICS
	++mp->mutex_set_wait;
#endif
	return (0);
#endif /* !HAVE_SPINLOCKS */
}

/*
 * __db_mutex_unlock --
 *	Release a lock.
 *
 * PUBLIC: int __db_mutex_unlock __P((db_mutex_t *, int));
 */
int
__db_mutex_unlock(mp, fd)
	db_mutex_t *mp;
	int fd;
{
#ifdef DEBUG
	if (mp->pid == 0) {
		(void)fprintf(stderr,
	    "MUTEX ERROR: __db_mutex_unlock: lock already unlocked\n");
		abort();
	}
#endif

#ifdef HAVE_SPINLOCKS
#ifdef DEBUG
	mp->pid = 0;
#endif

	/* Release the resource tsl. */
	TSL_UNSET(&mp->tsl_resource);
#else
	/*
	 * Release the resource tsl.  We don't have to acquire any locks
	 * because processes trying to acquire the lock are checking for
	 * a pid of 0, not a specific value.
	 */
	mp->pid = 0;
#endif
	return (0);
}
