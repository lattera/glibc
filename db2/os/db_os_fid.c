/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_os_fid.c	10.7 (Sleepycat) 8/21/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "os_ext.h"
#include "common_ext.h"

/*
 * __db_fileid --
 *	Return a unique identifier for a file.
 *
 * PUBLIC: int __db_fileid __P((DB_ENV *, const char *, int, u_int8_t *));
 */
int
__db_fileid(dbenv, fname, timestamp, fidp)
	DB_ENV *dbenv;
	const char *fname;
	int timestamp;
	u_int8_t *fidp;
{
	time_t now;
	u_int8_t *p;
	unsigned int i;

#ifdef _WIN32
	/*
	 * The documentation for GetFileInformationByHandle() states that the
	 * inode-type numbers are not constant between processes.  Actually,
	 * they are, they're the NTFS MFT indexes.  So, this works on NTFS,
	 * but perhaps not on other platforms, and perhaps not over a network.
	 * Can't think of a better solution right now.
	 */
	int fd = 0;
	HANDLE fh = 0;
	BY_HANDLE_FILE_INFORMATION fi;
	BOOL retval = FALSE;

	/* Clear the buffer. */
	memset(fidp, 0, DB_FILE_ID_LEN);

	/* first we open the file, because we're not given a handle to it */
	fd = open(fname,_O_RDONLY,_S_IREAD);
	if (-1 == fd) {
		/* If we can't open it, we're in trouble */
		return (errno);
	}

	/* File open, get its info */
	fh = (HANDLE)_get_osfhandle(fd);
	if ((HANDLE)(-1) != fh) {
		retval = GetFileInformationByHandle(fh,&fi);
	}
	close(fd);

	/*
	 * We want the three 32-bit words which tell us the volume ID and
	 * the file ID.  We make a crude attempt to copy the bytes over to
	 * the callers buffer.
	 *
	 * DBDB: really we should ensure that the bytes get packed the same
	 * way on all compilers, platforms etc.
	 */
	if ( ((HANDLE)(-1) != fh) && (TRUE == retval) ) {
		memcpy(fidp, &fi.nFileIndexLow, sizeof(u_int32_t));
		fidp += sizeof(u_int32_t);
		memcpy(fidp, &fi.nFileIndexHigh, sizeof(u_int32_t));
		fidp += sizeof(u_int32_t);
		memcpy(fidp, &fi.dwVolumeSerialNumber, sizeof(u_int32_t));
	}
#else
	struct stat sb;

	/* Clear the buffer. */
	memset(fidp, 0, DB_FILE_ID_LEN);

	/* Check for the unthinkable. */
	if (sizeof(sb.st_ino) +
	    sizeof(sb.st_dev) + sizeof(time_t) > DB_FILE_ID_LEN)
		return (EINVAL);

	/* On UNIX, use a dev/inode pair. */
	if (stat(fname, &sb)) {
		__db_err(dbenv, "%s: %s", fname, strerror(errno));
		return (errno);
	}

	/*
	 * Use the inode first and in reverse order, hopefully putting the
	 * distinguishing information early in the string.
	 */
	for (p = (u_int8_t *)&sb.st_ino +
	    sizeof(sb.st_ino), i = 0; i < sizeof(sb.st_ino); ++i)
		*fidp++ = *--p;
	for (p = (u_int8_t *)&sb.st_dev +
	    sizeof(sb.st_dev), i = 0; i < sizeof(sb.st_dev); ++i)
		*fidp++ = *--p;
#endif
	if (timestamp) {
		(void)time(&now);
		for (p = (u_int8_t *)&now +
		    sizeof(now), i = 0; i < sizeof(now); ++i)
			*fidp++ = *--p;
	}
	return (0);
}
