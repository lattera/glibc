/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_shash.c	10.3 (Sleepycat) 6/21/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "common_ext.h"

/* Powers-of-2 and close-by prime number pairs. */
static const struct {
	int	power;
	int	prime;
} list[] = {
	{  64,	  67},
	{ 128,	 131},
	{ 256,	 257},
	{ 512,	 521},
	{1024,	1031},
	{2048,	2053},
	{4096,	4099},
	{8192,	8191},
	{0,	   0}
};

/*
 * __db_tablesize --
 *	Choose a size for the hash table.
 *
 * PUBLIC: int __db_tablesize __P((int));
 */
int
__db_tablesize(n_buckets)
	int n_buckets;
{
	int i;

	/*
	 * We try to be clever about how big we make the hash tables.  Pick
	 * a prime number close to the "suggested" number of elements that
	 * will be in the hash table.  We shoot for minimum collisions (i.e.
	 * one element in each bucket).  We use 64 as the minimum table size.
	 *
	 * Ref: Sedgewick, Algorithms in C, "Hash Functions"
	 */
	if (n_buckets < 64)
		n_buckets = 64;

	for (i = 0;; ++i) {
		if (list[i].power == 0) {
			--i;
			break;
		}
		if (list[i].power >= n_buckets)
			break;
	}
	return (list[i].prime);
}

/*
 * __db_hashinit --
 *	Initialize a hash table that resides in shared memory.
 *
 * PUBLIC: void __db_hashinit __P((void *, int));
 */
void
__db_hashinit(begin, nelements)
	void *begin;
	int nelements;
{
	int i;
	SH_TAILQ_HEAD(hash_head) *headp;

	headp = (struct hash_head *)begin;

	for (i = 0; i < nelements; i++, headp++)
		SH_TAILQ_INIT(headp);
}
