/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)os.h	10.11 (Sleepycat) 10/12/98
 */

/*
 * We group seek/write calls into a single function so that we can use
 * pread(2)/pwrite(2) where they're available.
 */
#define	DB_IO_READ	1
#define	DB_IO_WRITE	2
typedef struct __io {
	int	    fd_io;		/* I/O file descriptor. */
	int	    fd_lock;		/* Locking file descriptor. */
	db_mutex_t *mutexp;		/* Mutex to lock. */
	size_t	    pagesize;		/* Page size. */
	db_pgno_t   pgno;		/* Page number. */
	u_int8_t   *buf;		/* Buffer. */
	size_t	    bytes;		/* Bytes read/written. */
} DB_IO;
