/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1995, 1996
 *	The President and Fellows of Harvard University.  All rights reserved.
 *
 * This code is derived from software contributed to Harvard by
 * Margo Seltzer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_region.c	10.15 (Sleepycat) 10/25/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "common_ext.h"

static int __db_rmap __P((DB_ENV *, int, size_t, void *));

/*
 * __db_rcreate --
 *
 * Common interface for creating a shared region.  Handles synchronization
 * across multiple processes.
 *
 * The dbenv contains the environment for this process, including naming
 * information.  The path argument represents the parameters passed to
 * the open routines and may be either a file or a directory.  If it is
 * a directory, it must exist.  If it is a file, then the file parameter
 * must be NULL, otherwise, file is the name to be created inside the
 * directory path.
 *
 * The function returns a pointer to the shared region that has been mapped
 * into memory, NULL on error.
 *
 * PUBLIC: int __db_rcreate __P((DB_ENV *, APPNAME,
 * PUBLIC:    const char *, const char *, int, size_t, int *, void *));
 */
int
__db_rcreate(dbenv, appname, path, file, mode, size, fdp, retp)
	DB_ENV *dbenv;
	APPNAME appname;
	const char *path, *file;
	int mode, *fdp;
	size_t size;
	void *retp;
{
	RLAYOUT *rp;
	int fd, ret;
	char *name;

	fd = -1;
	rp = NULL;

	/*
	 * Get the filename -- note, if it's a temporary file, it will
	 * be created by the underlying temporary file creation code,
	 * so we have to check the file descriptor to be sure it's an
	 * error.
	 */
	if ((ret = __db_appname(dbenv, appname, path, file, &fd, &name)) != 0)
		return (ret);

	/*
	 * Now open the file. We need to make sure that multiple processes
	 * that attempt to create the region at the same time are properly
	 * ordered, so we open it O_EXCL and O_CREAT so two simultaneous
	 * attempts to create the region will return failure in one of the
	 * attempts.
	 */
	if (fd == -1 && (ret = __db_open(name,
	    DB_CREATE | DB_EXCL, DB_CREATE | DB_EXCL, mode, &fd)) != 0) {
		if (ret != EEXIST)
			__db_err(dbenv,
			    "region create: %s: %s", name, strerror(ret));
		goto err;
	}
	*fdp = fd;

	/* Grow the region to the correct size. */
	if ((ret = __db_rgrow(dbenv, fd, size)) != 0)
		goto err;

	/* Map the region in. */
	if ((ret = __db_rmap(dbenv, fd, size, &rp)) != 0)
		goto err;

	/* Initialize the region. */
	if ((ret = __db_rinit(dbenv, rp, fd, size, 1)) != 0)
		goto err;

	if (name != NULL)
		FREES(name);

	*(void **)retp = rp;
	return (0);

err:	if (fd != -1) {
		if (rp != NULL)
			(void)__db_unmap(rp, rp->size);
		(void)__db_unlink(name);
		(void)__db_close(fd);
	}
	if (name != NULL)
		FREES(name);
	return (ret);
}

/*
 * __db_rinit --
 *	Initialize the region.
 *
 * PUBLIC: int __db_rinit __P((DB_ENV *, RLAYOUT *, int, size_t, int));
 */
int
__db_rinit(dbenv, rp, fd, size, lock_region)
	DB_ENV *dbenv;
	RLAYOUT *rp;
	size_t size;
	int fd, lock_region;
{
	int ret;

	/*
	 * Initialize the common information.
	 *
	 * !!!
	 * We have to order the region creates so that two processes don't try
	 * to simultaneously create the region and so that processes that are
	 * joining the region never see inconsistent data.  We'd like to play
	 * file permissions games, but we can't because WNT filesystems won't
	 * open a file mode 0.
	 *
	 * If the lock_region flag is set, the process creating the region
	 * acquires the lock before the setting the version number.  Any
	 * process joining the region checks the version number before
	 * attempting to acquire the lock.  (The lock_region flag may not be
	 * set -- the mpool code sometimes malloc's private regions but still
	 * needs to initialize them, specifically, the mutex for threads.)
	 *
	 * We have to check the version number first, because if the version
	 * number has not been written, it's possible that the mutex has not
	 * been initialized in which case an attempt to get it could lead to
	 * random behavior.  If the version number isn't there (the file size
	 * is too small) or it's 0, we know that the region is being created.
	 */
	__db_mutex_init(&rp->lock, MUTEX_LOCK_OFFSET(rp, &rp->lock));
	if (lock_region && (ret = __db_mutex_lock(&rp->lock, fd)) != 0)
		return (ret);

	rp->refcnt = 1;
	rp->size = size;
	rp->flags = 0;
	db_version(&rp->majver, &rp->minver, &rp->patch);

	return (0);
}

/*
 * __db_ropen --
 *	Construct the name of a file, open it and map it in.
 *
 * PUBLIC: int __db_ropen __P((DB_ENV *,
 * PUBLIC:    APPNAME, const char *, const char *, int, int *, void *));
 */
int
__db_ropen(dbenv, appname, path, file, flags, fdp, retp)
	DB_ENV *dbenv;
	APPNAME appname;
	const char *path, *file;
	int flags, *fdp;
	void *retp;
{
	RLAYOUT *rp;
	off_t size1, size2;
	int fd, ret;
	char *name;

	fd = -1;
	rp = NULL;

	/* Get the filename. */
	if ((ret = __db_appname(dbenv, appname, path, file, NULL, &name)) != 0)
		return (ret);

	/* Open the file. */
	if ((ret = __db_open(name, flags, DB_MUTEXDEBUG, 0, &fd)) != 0) {
		__db_err(dbenv, "region open: %s: %s", name, strerror(ret));
		goto err2;
	}

	*fdp = fd;

	/*
	 * Map the file in.  We have to do things in a strange order so that
	 * we don't get into a situation where the file was just created and
	 * isn't yet initialized.  See the comment in __db_rcreate() above.
	 *
	 * XXX
	 * We'd like to test to see if the file is too big to mmap.  Since we
	 * don't know what size or type off_t's or size_t's are, or the largest
	 * unsigned integral type is, or what random insanity the local C
	 * compiler will perpetrate, doing the comparison in a portable way is
	 * flatly impossible.  Hope that mmap fails if the file is too large.
	 *
	 */
	if ((ret = __db_ioinfo(name, fd, &size1, NULL)) != 0) {
		__db_err(dbenv, "%s: %s", name, strerror(ret));
		goto err2;
	}

	/* Check to make sure the first block has been written. */
	if ((size_t)size1 < sizeof(RLAYOUT)) {
		ret = EAGAIN;
		goto err2;
	}

	/* Map in whatever is there. */
	if ((ret = __db_rmap(dbenv, fd, size1, &rp)) != 0)
		goto err2;

	/*
	 * Check to make sure the region has been initialized.  We can't just
	 * grab the lock because the lock may not have been initialized yet.
	 */
	if (rp->majver == 0) {
		ret = EAGAIN;
		goto err2;
	}

	/* Get the region lock. */
	if (!LF_ISSET(DB_MUTEXDEBUG))
		(void)__db_mutex_lock(&rp->lock, fd);

	/*
	 * The file may have been half-written if we were descheduled between
	 * getting the size of the file and checking the major version.  Check
	 * to make sure we got the entire file.
	 */
	if ((ret = __db_ioinfo(name, fd, &size2, NULL)) != 0) {
		__db_err(dbenv, "%s: %s", name, strerror(ret));
		goto err1;
	}
	if (size1 != size2) {
		ret = EAGAIN;
		goto err1;
	}

	/* The file may have just been deleted. */
	if (F_ISSET(rp, DB_R_DELETED)) {
		ret = EAGAIN;
		goto err1;
	}

	/* Increment the reference count. */
	++rp->refcnt;

	/* Release the lock. */
	if (!LF_ISSET(DB_MUTEXDEBUG))
		(void)__db_mutex_unlock(&rp->lock, fd);

	FREES(name);

	*(void **)retp = rp;
	return (0);

err1:	if (!LF_ISSET(DB_MUTEXDEBUG))
		(void)__db_mutex_unlock(&rp->lock, fd);
err2:	if (rp != NULL)
		(void)__db_unmap(rp, rp->size);
	if (fd != -1)
		(void)__db_close(fd);
	FREES(name);
	return (ret);
}

/*
 * __db_rclose --
 *	Close a shared memory region.
 *
 * PUBLIC: int __db_rclose __P((DB_ENV *, int, void *));
 */
int
__db_rclose(dbenv, fd, ptr)
	DB_ENV *dbenv;
	int fd;
	void *ptr;
{
	RLAYOUT *rp;
	int ret, t_ret;
	const char *fail;

	rp = ptr;
	fail = NULL;

	/* Get the lock. */
	if ((ret = __db_mutex_lock(&rp->lock, fd)) != 0) {
		fail = "lock get";
		goto err;
	}

	/* Decrement the reference count. */
	--rp->refcnt;

	/* Release the lock. */
	if ((t_ret = __db_mutex_unlock(&rp->lock, fd)) != 0 && fail == NULL) {
		ret = t_ret;
		fail = "lock release";
	}

	/* Discard the region. */
	if ((t_ret = __db_unmap(ptr, rp->size)) != 0 && fail == NULL) {
		ret = t_ret;
		fail = "munmap";
	}

	if ((t_ret = __db_close(fd)) != 0 && fail == NULL) {
		ret = t_ret;
		fail = "close";
	}

	if (fail == NULL)
		return (0);

err:	__db_err(dbenv, "region detach: %s: %s", fail, strerror(ret));
	return (ret);
}

/*
 * __db_runlink --
 *	Remove a shared memory region.
 *
 * PUBLIC: int __db_runlink __P((DB_ENV *,
 * PUBLIC:    APPNAME, const char *, const char *, int));
 */
int
__db_runlink(dbenv, appname, path, file, force)
	DB_ENV *dbenv;
	APPNAME appname;
	const char *path, *file;
	int force;
{
	RLAYOUT *rp;
	int cnt, fd, ret, t_ret;
	char *name;

	rp = NULL;

	/* Get the filename. */
	if ((ret = __db_appname(dbenv, appname, path, file, NULL, &name)) != 0)
		return (ret);

	/* If the file doesn't exist, we're done. */
	if (__db_exists(name, NULL))
		return (0);		/* XXX: ENOENT? */

	/*
	 * If we're called with a force flag, try and unlink the file.  This
	 * may not succeed if the file is currently open, but there's nothing
	 * we can do about that.  There is a race condition between the check
	 * for existence above and the actual unlink.  If someone else snuck
	 * in and removed it before we do the remove, then we might get an
	 * ENOENT error.  If we get the ENOENT, we treat it as success, just
	 * as we do above.
	 */
	if (force) {
		if ((ret = __db_unlink(name)) != 0 && ret != ENOENT)
			goto err1;
		FREES(name);
		return (0);
	}

	/* Open and lock the region. */
	if ((ret = __db_ropen(dbenv, appname, path, file, 0, &fd, &rp)) != 0)
		goto err1;
	(void)__db_mutex_lock(&rp->lock, fd);

	/* If the region is currently being deleted, fail. */
	if (F_ISSET(rp, DB_R_DELETED)) {
		ret = ENOENT;		/* XXX: ENOENT? */
		goto err2;
	}

	/* If the region is currently in use by someone else, fail. */
	if (rp->refcnt > 1) {
		ret = EBUSY;
		goto err2;
	}

	/* Set the delete flag. */
	F_SET(rp, DB_R_DELETED);

	/* Release the lock and close the region. */
	(void)__db_mutex_unlock(&rp->lock, fd);
	if ((t_ret = __db_rclose(dbenv, fd, rp)) != 0 && ret == 0)
		goto err1;

	/*
	 * Unlink the region.  There's a race here -- other threads or
	 * processes might be opening the region while we're trying to
	 * remove it.  They'll fail, because we've set the DELETED flag,
	 * but they could still stop us from succeeding in the unlink.
	 */
	for (cnt = 5; cnt > 0; --cnt) {
		if ((ret = __db_unlink(name)) == 0)
			break;
		(void)__db_sleep(0, 250000);
	}
	if (ret == 0) {
		FREES(name);
		return (0);
	}

	/* Not a clue.  Try to clear the DB_R_DELETED flag. */
	if ((ret = __db_ropen(dbenv, appname, path, file, 0, &fd, &rp)) != 0)
		goto err1;
	(void)__db_mutex_lock(&rp->lock, fd);
	F_CLR(rp, DB_R_DELETED);
	/* FALLTHROUGH */

err2:	(void)__db_mutex_unlock(&rp->lock, fd);
	(void)__db_rclose(dbenv, fd, rp);
err1:	__db_err(dbenv, "region unlink: %s: %s", name, strerror(ret));
	FREES(name);
	return (ret);
}

/*
 * DB creates all regions on 4K boundaries so that we don't make the
 * underlying VM unhappy.
 */
#define	__DB_VMPAGESIZE	(4 * 1024)

/*
 * __db_rgrow --
 *	Extend a region by a specified amount.
 *
 * PUBLIC: int __db_rgrow __P((DB_ENV *, int, size_t));
 */
int
__db_rgrow(dbenv, fd, incr)
	DB_ENV *dbenv;
	int fd;
	size_t incr;
{
#ifdef MMAP_INIT_NEEDED
	size_t i;
#endif
	ssize_t nw;
	int ret;
	char buf[__DB_VMPAGESIZE];

	/* Seek to the end of the region. */
	if ((ret = __db_seek(fd, 0, 0, 0, SEEK_END)) != 0)
		goto err;

	/* Write nuls to the new bytes. */
	memset(buf, 0, sizeof(buf));

	/*
	 * Historically, some systems required that all of the bytes of the
	 * region be written before you could mmap it and access it randomly.
	 */
#ifdef MMAP_INIT_NEEDED
	/* Extend the region by writing each new page. */
	for (i = 0; i < incr; i += __DB_VMPAGESIZE) {
		if ((ret = __db_write(fd, buf, sizeof(buf), &nw)) != 0)
			goto err;
		if (nw != sizeof(buf))
			goto eio;
	}
#else
	/*
	 * Extend the region by writing the last page.
	 *
	 * Round off the increment to the next page boundary.
	 */
	incr += __DB_VMPAGESIZE - 1;
	incr -= incr % __DB_VMPAGESIZE;

	/* Write the last page, not the page after the last. */
	if ((ret = __db_seek(fd, 0, 0, incr - __DB_VMPAGESIZE, SEEK_CUR)) != 0)
		goto err;
	if ((ret = __db_write(fd, buf, sizeof(buf), &nw)) != 0)
		goto err;
	if (nw != sizeof(buf))
		goto eio;
#endif
	return (0);

eio:	ret = EIO;
err:	__db_err(dbenv, "region grow: %s", strerror(ret));
	return (ret);
}

/*
 * __db_rremap --
 *	Unmap the old region and map in a new region of a new size.  If
 *	either call fails, returns NULL, else returns the address of the
 *	new region.
 *
 * PUBLIC: int __db_rremap __P((DB_ENV *, void *, size_t, size_t, int, void *));
 */
int
__db_rremap(dbenv, ptr, oldsize, newsize, fd, retp)
	DB_ENV *dbenv;
	void *ptr, *retp;
	size_t oldsize, newsize;
	int fd;
{
	int ret;

	if ((ret = __db_unmap(ptr, oldsize)) != 0) {
		__db_err(dbenv, "region remap: munmap: %s", strerror(ret));
		return (ret);
	}

	return (__db_rmap(dbenv, fd, newsize, retp));
}

/*
 * __db_rmap --
 *	Attach to a shared memory region.
 */
static int
__db_rmap(dbenv, fd, size, retp)
	DB_ENV *dbenv;
	int fd;
	size_t size;
	void *retp;
{
	RLAYOUT *rp;
	int ret;

	if ((ret = __db_map(fd, size, 0, 0, (void **)&rp)) != 0) {
		__db_err(dbenv, "region map: mmap %s", strerror(ret));
		return (ret);
	}
	if (rp->size < size)
		rp->size = size;

	*(void **)retp = rp;
	return (0);
}
