/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_mmap.c	10.4 (Sleepycat) 6/28/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/mman.h>

#include <errno.h>
#endif

#include "db_int.h"
#include "os_ext.h"

/*
 * __db_mmap --
 *	Map in some shared memory backed by a file descriptor.
 *
 * PUBLIC: int __db_mmap __P((int, size_t, int, int, void *));
 */
int
__db_mmap(fd, len, is_private, rdonly, addr)
	int fd, is_private, rdonly;
	size_t len;
	void *addr;
{
#ifdef _WIN32
	/* We have not implemented copy-on-write here */
	void * pMemory = 0;
	HANDLE hFile = (HANDLE)_get_osfhandle(fd);
	HANDLE hMemory = CreateFileMapping(
	      hFile,
	      0,
	      (rdonly ? PAGE_READONLY : PAGE_READWRITE),
	      0,
	      len, /* This code fails if the library is ever compiled on a 64-bit machine */
	      0
	      );
	if (NULL == hMemory)
	{
	      return errno;
	}
	pMemory = MapViewOfFile(
	      hMemory,
	      (rdonly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS),
	      0,
	      0,
	      len
	      );
	CloseHandle(hMemory);
	*(void **)addr = pMemory;
	return 0;

#else /* !_WIN32 */

	void *p;
	int flags, prot;

	flags = is_private ? MAP_PRIVATE : MAP_SHARED;
#ifdef MAP_HASSEMAPHORE
	flags += MAP_HASSEMAPHORE;
#endif
	prot = PROT_READ | (rdonly ? 0 : PROT_WRITE);

#ifndef MAP_FAILED			/* XXX: Mmap(2) failure return. */
#define	MAP_FAILED	-1
#endif
	if ((p =
	    mmap(NULL, len, prot, flags, fd, (off_t)0)) == (void *)MAP_FAILED)
		return (errno);

	*(void **)addr = p;
	return (0);
#endif /* _WIN32 */
}

/*
 * __db_unmap --
 *	Release the specified shared memory.
 *
 * PUBLIC: int __db_munmap __P((void *, size_t));
 */
int
__db_munmap(addr, len)
	void *addr;
	size_t len;
{
	/*
	 * !!!
	 * The argument len is always the same length as was mapped.
	 */
#ifdef _WIN32
	return (!UnmapViewOfFile(addr) ? errno : 0);
#else
	return (munmap(addr, len) ? errno : 0);
#endif
}
