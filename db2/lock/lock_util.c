/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)lock_util.c	10.4 (Sleepycat) 7/22/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "db_page.h"
#include "db_shash.h"
#include "hash.h"
#include "lock.h"

/*
 * This function is used to compare a DBT that is about to be entered
 * into a hash table with an object already in the hash table.  Note
 * that it just returns true on equal and 0 on not-equal.  Therefore this
 * cannot be used as a sort function; its purpose is to be used as a
 * hash comparison function.
 * PUBLIC: int __lock_cmp __P((DBT *, DB_LOCKOBJ *));
 */
int
__lock_cmp(dbt, lock_obj)
	DBT *dbt;
	DB_LOCKOBJ *lock_obj;
{
	void *obj_data;

	if (lock_obj->type != DB_LOCK_OBJTYPE)
		return (0);
	obj_data = SH_DBT_PTR(&lock_obj->lockobj);
	return (dbt->size == lock_obj->lockobj.size &&
		memcmp(dbt->data, obj_data, dbt->size) == 0);
}

/*
 * PUBLIC: int __lock_locker_cmp __P((u_int32_t, DB_LOCKOBJ *));
 */
int
__lock_locker_cmp(locker, lock_obj)
	u_int32_t locker;
	DB_LOCKOBJ *lock_obj;
{
	void *obj_data;

	if (lock_obj->type != DB_LOCK_LOCKER)
		return (0);

	obj_data = SH_DBT_PTR(&lock_obj->lockobj);
	return (memcmp(&locker, obj_data, sizeof(u_int32_t)) == 0);
}

/*
 * PUBLIC: int __lock_ohash __P((DBT *));
 */
int
__lock_ohash(dbt)
	DBT *dbt;
{
	return (__ham_func5(dbt->data, dbt->size));
}

/*
 * PUBLIC: u_int32_t __lock_locker_hash __P((u_int32_t));
 */
u_int32_t
__lock_locker_hash(locker)
	u_int32_t locker;
{
	return (__ham_func5(&locker, sizeof(locker)));
}

/*
 * PUBLIC: u_int32_t __lock_lhash __P((DB_LOCKOBJ *));
 */
u_int32_t
__lock_lhash(lock_obj)
	DB_LOCKOBJ *lock_obj;
{
	void *obj_data;

	obj_data = SH_DBT_PTR(&lock_obj->lockobj);
	return (__ham_func5(obj_data, lock_obj->lockobj.size));
}

