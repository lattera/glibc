/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_map.c	10.7 (Sleepycat) 10/25/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/mman.h>

#include <errno.h>
#endif

#include "db_int.h"

/*
 * __os_map --
 *	Map in some shared memory backed by a file descriptor.
 *
 * PUBLIC: int __os_map __P((int, size_t, int, int, void **));
 */
int
__os_map(fd, len, is_private, is_rdonly, addr)
	int fd, is_private, is_rdonly;
	size_t len;
	void **addr;
{
	void *p;
	int flags, prot;

	flags = is_private ? MAP_PRIVATE : MAP_SHARED;
#ifdef MAP_HASSEMAPHORE
	flags |= MAP_HASSEMAPHORE;
#endif
	prot = PROT_READ | (is_rdonly ? 0 : PROT_WRITE);

#ifndef MAP_FAILED			/* XXX: Mmap(2) failure return. */
#define	MAP_FAILED	-1
#endif
	if ((p =
	    mmap(NULL, len, prot, flags, fd, (off_t)0)) == (void *)MAP_FAILED)
		return (errno);

	*addr = p;
	return (0);
}

/*
 * __os_unmap --
 *	Release the specified shared memory.
 *
 * PUBLIC: int __os_unmap __P((void *, size_t));
 */
int
__os_unmap(addr, len)
	void *addr;
	size_t len;
{
	/*
	 * !!!
	 * The argument len is always the same length as was mapped.
	 */
	return (munmap(addr, len) ? errno : 0);
}
