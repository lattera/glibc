/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_lseek.c	10.3 (Sleepycat) 6/28/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_ext.h"

/*
 * __db_lseek --
 *	Seek to a page/byte offset in the file.
 *
 * PUBLIC: int __db_lseek __P((int, size_t, db_pgno_t, u_long, int));
 */
int
__db_lseek(fd, pgsize, pageno, relative, whence)
	int fd;
	size_t pgsize;
	db_pgno_t pageno;
	u_long relative;
	int whence;
{
	/* 64-bit offsets are done differently by different vendors. */
#undef	__LSEEK_SET
#ifdef	HAVE_LLSEEK
#define	__LSEEK_SET
	offset_t offset;			/* Solaris. */

	offset = pgsize * pageno + relative;
	return (llseek(fd, offset, whence) == -1 ? errno : 0);
#endif
#ifdef	HAVE_LSEEKI
#define	__LSEEK_SET
	__int64 offset;				/* WNT */

	offset = pgsize * pageno + relative;
	return (_lseeki64(fd, offset, whence) == -1 ? errno : 0);
#endif
#ifndef	__LSEEK_SET
	off_t offset;				/* Default. */

	offset = pgsize * pageno + relative;
	return (lseek(fd, offset, whence) == -1 ? errno : 0);
#endif
}
