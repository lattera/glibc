/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_rw.c	10.11 (Sleepycat) 10/12/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * __os_io --
 *	Do an I/O.
 *
 * PUBLIC: int __os_io __P((DB_IO *, int, ssize_t *));
 */
int
__os_io(db_iop, op, niop)
	DB_IO *db_iop;
	int op;
	ssize_t *niop;
{
	int ret;

#ifdef HAVE_PREAD
	switch (op) {
	case DB_IO_READ:
		if (__db_jump.j_read != NULL)
			goto slow;
		*niop = pread(db_iop->fd_io, db_iop->buf,
		    db_iop->bytes, (off_t)db_iop->pgno * db_iop->pagesize);
		break;
	case DB_IO_WRITE:
		if (__db_jump.j_write != NULL)
			goto slow;
		*niop = pwrite(db_iop->fd_io, db_iop->buf,
		    db_iop->bytes, (off_t)db_iop->pgno * db_iop->pagesize);
		break;
	}
	if (*niop == db_iop->bytes)
		return (0);
slow:
#endif
	if (db_iop->mutexp != NULL)
		(void)__db_mutex_lock(db_iop->mutexp, db_iop->fd_lock);

	if ((ret = __os_seek(db_iop->fd_io,
	    db_iop->pagesize, db_iop->pgno, 0, 0, SEEK_SET)) != 0)
		goto err;
	switch (op) {
	case DB_IO_READ:
		ret =
		    __os_read(db_iop->fd_io, db_iop->buf, db_iop->bytes, niop);
		break;
	case DB_IO_WRITE:
		ret =
		    __os_write(db_iop->fd_io, db_iop->buf, db_iop->bytes, niop);
		break;
	}

err:	if (db_iop->mutexp != NULL)
		(void)__db_mutex_unlock(db_iop->mutexp, db_iop->fd_lock);

	return (ret);

}

/*
 * __os_read --
 *	Read from a file handle.
 *
 * PUBLIC: int __os_read __P((int, void *, size_t, ssize_t *));
 */
int
__os_read(fd, addr, len, nrp)
	int fd;
	void *addr;
	size_t len;
	ssize_t *nrp;
{
	size_t offset;
	ssize_t nr;
	u_int8_t *taddr;

	for (taddr = addr,
	    offset = 0; offset < len; taddr += nr, offset += nr) {
		if ((nr = __db_jump.j_read != NULL ?
		    __db_jump.j_read(fd, taddr, len - offset) :
		    read(fd, taddr, len - offset)) < 0)
			return (errno);
		if (nr == 0)
			break;
	}
	*nrp = taddr - (u_int8_t *)addr;
	return (0);
}

/*
 * __os_write --
 *	Write to a file handle.
 *
 * PUBLIC: int __os_write __P((int, void *, size_t, ssize_t *));
 */
int
__os_write(fd, addr, len, nwp)
	int fd;
	const void *addr;
	size_t len;
	ssize_t *nwp;
{
	size_t offset;
	ssize_t nw;
	u_int8_t *taddr;

	for (taddr = addr,
	    offset = 0; offset < len; taddr += nw, offset += nw)
		if ((nw = __db_jump.j_write != NULL ?
		    __db_jump.j_write(fd, taddr, len - offset) :
		    write(fd, taddr, len - offset)) < 0)
			return (errno);
	*nwp = len;
	return (0);
}
