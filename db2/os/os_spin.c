/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_spin.c	10.3 (Sleepycat) 11/25/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <limits.h>
#include <unistd.h>
#endif

#include "db_int.h"

/*
 * __os_spin --
 *	Return the number of default spins before blocking.
 *
 * PUBLIC: int __os_spin __P((void));
 */
int
__os_spin()
{
	extern int __db_tsl_spins;

	/* If the application specified the spins, use its value. */
	if (__db_tsl_spins != 0)
		return (__db_tsl_spins);

	/*
	 * XXX
	 * Sysconf: Solaris uses _SC_NPROCESSORS_ONLN to return the number
	 * of online processors.  I don't know if this call is portable or
	 * not.
	 */
#if defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
	{
		long sys_val;

		sys_val = sysconf(_SC_NPROCESSORS_ONLN);
		if (sys_val > 0)
			return (sys_val * 50);
	}
#endif

	/* Default to a single processor. */
	return (1);
}
