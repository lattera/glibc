/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)lock.c	10.36 (Sleepycat) 9/24/97";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <errno.h>
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
#include "lock.h"
#include "common_ext.h"
#include "db_am.h"

static void __lock_checklocker __P((DB_LOCKTAB *, struct __db_lock *, int));
static int  __lock_count_locks __P((DB_LOCKREGION *));
static int  __lock_count_objs __P((DB_LOCKREGION *));
static int  __lock_create __P((const char *, int, DB_ENV *));
static void __lock_freeobj __P((DB_LOCKTAB *, DB_LOCKOBJ *));
static int  __lock_get_internal __P((DB_LOCKTAB *, u_int32_t, int, const DBT *,
    db_lockmode_t, struct __db_lock **));
static int  __lock_grow_region __P((DB_LOCKTAB *, int, size_t));
static int  __lock_put_internal __P((DB_LOCKTAB *, struct __db_lock *, int));
static void __lock_remove_waiter
    __P((DB_LOCKTAB *, DB_LOCKOBJ *, struct __db_lock *, db_status_t));
static void __lock_reset_region __P((DB_LOCKTAB *));
static int  __lock_validate_region __P((DB_LOCKTAB *));
#ifdef DEBUG
static void __lock_dump_locker __P((DB_LOCKTAB *, DB_LOCKOBJ *));
static void __lock_dump_object __P((DB_LOCKTAB *, DB_LOCKOBJ *));
static void __lock_printlock __P((DB_LOCKTAB *, struct __db_lock *, int));
#endif

/*
 * Create and initialize a lock region in shared memory.
 */

/*
 * __lock_create --
 *	Create the lock region.  Returns an errno.  In most cases,
 * the errno should be that returned by __db_ropen, in which case
 * an EAGAIN means that we should retry, and an EEXIST means that
 * the region exists and we didn't need to create it.  Any other
 * sort of errno should be treated as a system error, leading to a
 * failure of the original interface call.
 */
static int
__lock_create(path, mode, dbenv)
	const char *path;
	int mode;
	DB_ENV *dbenv;
{
	struct __db_lock *lp;
	struct lock_header *tq_head;
	struct obj_header *obj_head;
	DB_LOCKOBJ *op;
	DB_LOCKREGION *lrp;
	u_int maxlocks;
	u_int32_t i;
	int fd, lock_modes, nelements, ret;
	u_int8_t *conflicts, *curaddr;

	maxlocks = dbenv == NULL || dbenv->lk_max == 0 ?
	    DB_LOCK_DEFAULT_N : dbenv->lk_max;
	lock_modes = dbenv == NULL || dbenv->lk_modes == 0 ?
	    DB_LOCK_RW_N : dbenv->lk_modes;
	conflicts = dbenv == NULL || dbenv->lk_conflicts == NULL ?
	    (u_int8_t *)db_rw_conflicts : dbenv->lk_conflicts;

	if ((ret =
	    __db_rcreate(dbenv, DB_APP_NONE, path, DB_DEFAULT_LOCK_FILE, mode,
	    LOCK_REGION_SIZE(lock_modes, maxlocks, __db_tablesize(maxlocks)),
	    &fd, &lrp)) != 0)
		return (ret);

	/* Region exists; now initialize it. */
	lrp->table_size = __db_tablesize(maxlocks);
	lrp->magic = DB_LOCKMAGIC;
	lrp->version = DB_LOCKVERSION;
	lrp->id = 0;
	lrp->maxlocks = maxlocks;
	lrp->need_dd = 0;
	lrp->detect = DB_LOCK_NORUN;
	lrp->numobjs = maxlocks;
	lrp->nlockers = 0;
	lrp->mem_bytes = ALIGN(STRING_SIZE(maxlocks), sizeof(size_t));
	lrp->increment = lrp->hdr.size / 2;
	lrp->nmodes = lock_modes;
	lrp->nconflicts = 0;
	lrp->nrequests = 0;
	lrp->nreleases = 0;
	lrp->ndeadlocks = 0;

	/*
	 * As we write the region, we've got to maintain the alignment
	 * for the structures that follow each chunk.  This information
	 * ends up being encapsulated both in here as well as in the
	 * lock.h file for the XXX_SIZE macros.
	 */
	/* Initialize conflict matrix. */
	curaddr = (u_int8_t *)lrp + sizeof(DB_LOCKREGION);
	memcpy(curaddr, conflicts, lock_modes * lock_modes);
	curaddr += lock_modes * lock_modes;

	/*
	 * Initialize hash table.
	 */
	curaddr = (u_int8_t *)ALIGNP(curaddr, LOCK_HASH_ALIGN);
	lrp->hash_off = curaddr - (u_int8_t *)lrp;
	nelements = lrp->table_size;
	__db_hashinit(curaddr, nelements);
	curaddr += nelements * sizeof(DB_HASHTAB);

	/*
	 * Initialize locks onto a free list. Since locks contains mutexes,
	 * we need to make sure that each lock is aligned on a MUTEX_ALIGNMENT
	 * boundary.
	 */
	curaddr = (u_int8_t *)ALIGNP(curaddr, MUTEX_ALIGNMENT);
	tq_head = &lrp->free_locks;
	SH_TAILQ_INIT(tq_head);

	for (i = 0; i++ < maxlocks;
	    curaddr += ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT)) {
		lp = (struct __db_lock *)curaddr;
		lp->status = DB_LSTAT_FREE;
		SH_TAILQ_INSERT_HEAD(tq_head, lp, links, __db_lock);
	}

	/* Initialize objects onto a free list.  */
	obj_head = &lrp->free_objs;
	SH_TAILQ_INIT(obj_head);

	for (i = 0; i++ < maxlocks; curaddr += sizeof(DB_LOCKOBJ)) {
		op = (DB_LOCKOBJ *)curaddr;
		SH_TAILQ_INSERT_HEAD(obj_head, op, links, __db_lockobj);
	}

	/*
	 * Initialize the string space; as for all shared memory allocation
	 * regions, this requires size_t alignment, since we store the
	 * lengths of malloc'd areas in the area..
	 */
	curaddr = (u_int8_t *)ALIGNP(curaddr, sizeof(size_t));
	lrp->mem_off = curaddr - (u_int8_t *)lrp;
	__db_shalloc_init(curaddr, lrp->mem_bytes);

	/* Release the lock. */
	(void)__db_mutex_unlock(&lrp->hdr.lock, fd);

	/* Now unmap the region. */
	if ((ret = __db_rclose(dbenv, fd, lrp)) != 0) {
		(void)lock_unlink(path, 1 /* force */, dbenv);
		return (ret);
	}

	return (0);
}

int
lock_open(path, flags, mode, dbenv, ltp)
	const char *path;
	int flags, mode;
	DB_ENV *dbenv;
	DB_LOCKTAB **ltp;
{
	DB_LOCKTAB *lt;
	int ret, retry_cnt;

	/* Validate arguments. */
#ifdef HAVE_SPINLOCKS
#define	OKFLAGS	(DB_CREATE | DB_THREAD)
#else
#define	OKFLAGS	(DB_CREATE)
#endif
	if ((ret = __db_fchk(dbenv, "lock_open", flags, OKFLAGS)) != 0)
		return (ret);

	/*
	 * Create the lock table structure.
	 */
	if ((lt = (DB_LOCKTAB *)calloc(1, sizeof(DB_LOCKTAB))) == NULL) {
		__db_err(dbenv, "%s", strerror(ENOMEM));
		return (ENOMEM);
	}
	lt->dbenv = dbenv;

	/*
	 * Now, create the lock region if it doesn't already exist.
	 */
	retry_cnt = 0;
retry:	if (LF_ISSET(DB_CREATE) &&
	    (ret = __lock_create(path, mode, dbenv)) != 0)
		if (ret == EAGAIN && ++retry_cnt < 3) {
			(void)__db_sleep(1, 0);
			goto retry;
		} else if (ret == EEXIST) /* We did not create the region */
			LF_CLR(DB_CREATE);
		else
			goto out;

	/*
	 * Finally, open the region, map it in, and increment the
	 * reference count.
	 */
	retry_cnt = 0;
retry1:	if ((ret = __db_ropen(dbenv, DB_APP_NONE, path, DB_DEFAULT_LOCK_FILE,
	    LF_ISSET(~(DB_CREATE | DB_THREAD)), &lt->fd, &lt->region)) != 0) {
		if (ret == EAGAIN && ++retry_cnt < 3) {
			(void)__db_sleep(1, 0);
			goto retry1;
		}
		goto out;
	 }

	if (lt->region->magic != DB_LOCKMAGIC) {
		__db_err(dbenv, "lock_open: Bad magic number");
		ret = EINVAL;
		goto out;
	}

	/* Check for automatic deadlock detection. */
	if (dbenv->lk_detect != DB_LOCK_NORUN) {
		if (lt->region->detect != DB_LOCK_NORUN &&
		    dbenv->lk_detect != DB_LOCK_DEFAULT &&
		    lt->region->detect != dbenv->lk_detect) {
			__db_err(dbenv,
			    "lock_open: incompatible deadlock detector mode");
			ret = EINVAL;
			goto out;
		}
		if (lt->region->detect == DB_LOCK_NORUN)
			lt->region->detect = dbenv->lk_detect;
	}

	/* Set up remaining pointers into region. */
	lt->conflicts = (u_int8_t *)lt->region + sizeof(DB_LOCKREGION);
	lt->hashtab =
	    (DB_HASHTAB *)((u_int8_t *)lt->region + lt->region->hash_off);
	lt->mem = (void *)((u_int8_t *)lt->region + lt->region->mem_off);
	lt->reg_size = lt->region->hdr.size;

	*ltp = lt;
	return (0);

/* Error handling. */
out:	if (lt->region != NULL)
		(void)__db_rclose(lt->dbenv, lt->fd, lt->region);
	if (LF_ISSET(DB_CREATE))
		(void)lock_unlink(path, 1, lt->dbenv);
	free(lt);
	return (ret);
}

int
lock_id (lt, idp)
	DB_LOCKTAB *lt;
	u_int32_t *idp;
{
	u_int32_t id;

	LOCK_LOCKREGION(lt);
	if (lt->region->id >= DB_LOCK_MAXID)
		lt->region->id = 0;
	id = ++lt->region->id;
	UNLOCK_LOCKREGION(lt);

	*idp = id;
	return (0);
}

int
lock_vec(lt, locker, flags, list, nlist, elistp)
	DB_LOCKTAB *lt;
	u_int32_t locker;
	int flags, nlist;
	DB_LOCKREQ *list, **elistp;
{
	struct __db_lock *lp;
	DB_LOCKOBJ *sh_obj, *sh_locker;
	int i, ret, run_dd;

	/* Validate arguments. */
	if ((ret =
	    __db_fchk(lt->dbenv, "lock_vec", flags, DB_LOCK_NOWAIT)) != 0)
		return (ret);

	LOCK_LOCKREGION(lt);

	if ((ret = __lock_validate_region(lt)) != 0) {
		UNLOCK_LOCKREGION(lt);
		return (ret);
	}

	ret = 0;
	for (i = 0; i < nlist && ret == 0; i++) {
		switch (list[i].op) {
		case DB_LOCK_GET:
			ret = __lock_get_internal(lt, locker, flags,
			    list[i].obj, list[i].mode, &lp);
			if (ret == 0) {
				list[i].lock = LOCK_TO_OFFSET(lt, lp);
				lt->region->nrequests++;
			}
			break;
		case DB_LOCK_PUT:
			lp = OFFSET_TO_LOCK(lt, list[i].lock);
			if (lp->holder != locker) {
				ret = DB_LOCK_NOTHELD;
				break;
			}
			list[i].mode = lp->mode;

			/* XXX Need to copy the object. ??? */
			ret = __lock_put_internal(lt, lp, 0);
			break;
		case DB_LOCK_PUT_ALL:
			/* Find the locker. */
			if ((ret = __lock_getobj(lt, locker,
			    NULL, DB_LOCK_LOCKER, &sh_locker)) != 0)
				break;

			for (lp = SH_LIST_FIRST(&sh_locker->heldby, __db_lock);
			    lp != NULL;
			    lp = SH_LIST_FIRST(&sh_locker->heldby, __db_lock)) {
				if ((ret = __lock_put_internal(lt, lp, 0)) != 0)
					break;
			}
			__lock_freeobj(lt, sh_locker);
			lt->region->nlockers--;
			break;
		case DB_LOCK_PUT_OBJ:

			/* Look up the object in the hash table. */
			HASHLOOKUP(lt->hashtab, __db_lockobj, links,
			    list[i].obj, sh_obj, lt->region->table_size,
			    __lock_ohash, __lock_cmp);
			if (sh_obj == NULL) {
				ret = EINVAL;
				break;
			}
			/*
			 * Release waiters first, because they won't cause
			 * anyone else to be awakened.  If we release the
			 * lockers first, all the waiters get awakened
			 * needlessly.
			 */
			for (lp = SH_TAILQ_FIRST(&sh_obj->waiters, __db_lock);
			    lp != NULL;
			    lp = SH_TAILQ_FIRST(&sh_obj->waiters, __db_lock)) {
				lt->region->nreleases += lp->refcount;
				__lock_remove_waiter(lt, sh_obj, lp,
				    DB_LSTAT_NOGRANT);
				__lock_checklocker(lt, lp, 1);
			}

			for (lp = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock);
			    lp != NULL;
			    lp = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock)) {

				lt->region->nreleases += lp->refcount;
				SH_LIST_REMOVE(lp, locker_links, __db_lock);
				SH_TAILQ_REMOVE(&sh_obj->holders, lp, links,
				    __db_lock);
				lp->status = DB_LSTAT_FREE;
				SH_TAILQ_INSERT_HEAD(&lt->region->free_locks,
				    lp, links, __db_lock);
			}

			/* Now free the object. */
			__lock_freeobj(lt, sh_obj);
			break;
#ifdef DEBUG
		case DB_LOCK_DUMP:
			/* Find the locker. */
			if ((ret = __lock_getobj(lt, locker,
			    NULL, DB_LOCK_LOCKER, &sh_locker)) != 0)
				break;

			for (lp = SH_LIST_FIRST(&sh_locker->heldby, __db_lock);
			    lp != NULL;
			    lp = SH_LIST_NEXT(lp, locker_links, __db_lock)) {
				__lock_printlock(lt, lp, 1);
				ret = EINVAL;
			}
			if (ret == 0) {
				__lock_freeobj(lt, sh_locker);
				lt->region->nlockers--;
			}
			break;
#endif
		default:
			ret = EINVAL;
			break;
		}
	}

	if (lt->region->need_dd && lt->region->detect != DB_LOCK_NORUN) {
		run_dd = 1;
		lt->region->need_dd = 0;
	} else
		run_dd = 0;

	UNLOCK_LOCKREGION(lt);

	if (ret == 0 && run_dd)
		lock_detect(lt, 0, lt->region->detect);

	if (elistp && ret != 0)
		*elistp = &list[i - 1];
	return (ret);
}

int
lock_get(lt, locker, flags, obj, lock_mode, lock)
	DB_LOCKTAB *lt;
	u_int32_t locker;
	int flags;
	const DBT *obj;
	db_lockmode_t lock_mode;
	DB_LOCK *lock;
{
	struct __db_lock *lockp;
	int ret;

	/* Validate arguments. */
	if ((ret =
	    __db_fchk(lt->dbenv, "lock_get", flags, DB_LOCK_NOWAIT)) != 0)
		return (ret);

	LOCK_LOCKREGION(lt);

	ret = __lock_validate_region(lt);
	if (ret == 0 && (ret = __lock_get_internal(lt,
	    locker, flags, obj, lock_mode, &lockp)) == 0) {
		*lock = LOCK_TO_OFFSET(lt, lockp);
		lt->region->nrequests++;
	}

	UNLOCK_LOCKREGION(lt);
	return (ret);
}

int
lock_put(lt, lock)
	DB_LOCKTAB *lt;
	DB_LOCK lock;
{
	struct __db_lock *lockp;
	int ret, run_dd;

	LOCK_LOCKREGION(lt);

	if ((ret = __lock_validate_region(lt)) != 0)
		return (ret);
	else {
		lockp = OFFSET_TO_LOCK(lt, lock);
		ret = __lock_put_internal(lt, lockp, 0);
	}

	__lock_checklocker(lt, lockp, 0);

	if (lt->region->need_dd && lt->region->detect != DB_LOCK_NORUN) {
		run_dd = 1;
		lt->region->need_dd = 0;
	} else
		run_dd = 0;

	UNLOCK_LOCKREGION(lt);

	if (ret == 0 && run_dd)
		lock_detect(lt, 0, lt->region->detect);

	return (ret);
}

int
lock_close(lt)
	DB_LOCKTAB *lt;
{
	int ret;

	if ((ret = __db_rclose(lt->dbenv, lt->fd, lt->region)) != 0)
		return (ret);

	/* Free lock table. */
	free(lt);
	return (0);
}

int
lock_unlink(path, force, dbenv)
	const char *path;
	int force;
	DB_ENV *dbenv;
{
	return (__db_runlink(dbenv,
	    DB_APP_NONE, path, DB_DEFAULT_LOCK_FILE, force));
}

/*
 * XXX This looks like it could be void, but I'm leaving it returning
 * an int because I think it will have to when we go through and add
 * the appropriate error checking for the EINTR on mutexes.
 */
static int
__lock_put_internal(lt, lockp, do_all)
	DB_LOCKTAB *lt;
	struct __db_lock *lockp;
	int do_all;
{
	struct __db_lock *lp_w, *lp_h, *next_waiter;
	DB_LOCKOBJ *sh_obj;
	int state_changed;

	if (lockp->refcount == 0 || (lockp->status != DB_LSTAT_HELD &&
	    lockp->status != DB_LSTAT_WAITING) || lockp->obj == 0) {
		__db_err(lt->dbenv, "lock_put: invalid lock %lu",
		    (u_long)((u_int8_t *)lockp - (u_int8_t *)lt->region));
		return (EINVAL);
	}

	if (do_all)
		lt->region->nreleases += lockp->refcount;
	else
		lt->region->nreleases++;
	if (do_all == 0 && lockp->refcount > 1) {
		lockp->refcount--;
		return (0);
	}

	/* Get the object associated with this lock. */
	sh_obj = (DB_LOCKOBJ *)((u_int8_t *)lockp + lockp->obj);

	/* Remove lock from locker list. */
	SH_LIST_REMOVE(lockp, locker_links, __db_lock);

	/* Remove this lock from its holders/waitlist. */
	if (lockp->status != DB_LSTAT_HELD)
		__lock_remove_waiter(lt, sh_obj, lockp, DB_LSTAT_FREE);
	else
		SH_TAILQ_REMOVE(&sh_obj->holders, lockp, links, __db_lock);

	/*
	 * We need to do lock promotion.  We also need to determine if
	 * we're going to need to run the deadlock detector again.  If
	 * we release locks, and there are waiters, but no one gets promoted,
	 * then we haven't fundamentally changed the lockmgr state, so
	 * we may still have a deadlock and we have to run again.  However,
	 * if there were no waiters, or we actually promoted someone, then
	 * we are OK and we don't have to run it immediately.
	 */
	for (lp_w = SH_TAILQ_FIRST(&sh_obj->waiters, __db_lock),
	    state_changed = lp_w == NULL;
	    lp_w != NULL;
	    lp_w = next_waiter) {
		next_waiter = SH_TAILQ_NEXT(lp_w, links, __db_lock);
		for (lp_h = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock);
		    lp_h != NULL;
		    lp_h = SH_TAILQ_NEXT(lp_h, links, __db_lock)) {
			if (CONFLICTS(lt, lp_h->mode, lp_w->mode) &&
			    lp_h->holder != lp_w->holder)
				break;
		}
		if (lp_h != NULL)	/* Found a conflict. */
			break;

		/* No conflict, promote the waiting lock. */
		SH_TAILQ_REMOVE(&sh_obj->waiters, lp_w, links, __db_lock);
		lp_w->status = DB_LSTAT_PENDING;
		SH_TAILQ_INSERT_TAIL(&sh_obj->holders, lp_w, links);

		/* Wake up waiter. */
		(void)__db_mutex_unlock(&lp_w->mutex, lt->fd);
		state_changed = 1;
	}

	/* Check if object should be reclaimed. */
	if (SH_TAILQ_FIRST(&sh_obj->holders, __db_lock) == NULL) {
		HASHREMOVE_EL(lt->hashtab, __db_lockobj,
		    links, sh_obj, lt->region->table_size, __lock_lhash);
		__db_shalloc_free(lt->mem, SH_DBT_PTR(&sh_obj->lockobj));
		SH_TAILQ_INSERT_HEAD(&lt->region->free_objs, sh_obj, links,
		    __db_lockobj);
		state_changed = 1;
	}

	/* Free lock. */
	lockp->status = DB_LSTAT_FREE;
	SH_TAILQ_INSERT_HEAD(&lt->region->free_locks, lockp, links, __db_lock);

	/*
	 * If we did not promote anyone; we need to run the deadlock
	 * detector again.
	 */
	if (state_changed == 0)
		lt->region->need_dd = 1;

	return (0);
}

static int
__lock_get_internal(lt, locker, flags, obj, lock_mode, lockp)
	DB_LOCKTAB *lt;
	u_int32_t locker;
	int flags;
	const DBT *obj;
	db_lockmode_t lock_mode;
	struct __db_lock **lockp;
{
	struct __db_lock *newl, *lp;
	DB_LOCKOBJ *sh_obj, *sh_locker;
	DB_LOCKREGION *lrp;
	size_t newl_off;
	int ret;

	ret = 0;
	/*
	 * Check that lock mode is valid.
	 */

	lrp = lt->region;
	if ((u_int32_t)lock_mode >= lrp->nmodes) {
		__db_err(lt->dbenv,
		    "lock_get: invalid lock mode %lu\n", (u_long)lock_mode);
		return (EINVAL);
	}

	/* Allocate a new lock.  Optimize for the common case of a grant. */
	if ((newl = SH_TAILQ_FIRST(&lrp->free_locks, __db_lock)) == NULL) {
		if ((ret = __lock_grow_region(lt, DB_LOCK_LOCK, 0)) != 0)
			return (ret);
		lrp = lt->region;
		newl = SH_TAILQ_FIRST(&lrp->free_locks, __db_lock);
	}
	newl_off = LOCK_TO_OFFSET(lt, newl);

	/* Optimize for common case of granting a lock. */
	SH_TAILQ_REMOVE(&lrp->free_locks, newl, links, __db_lock);

	newl->mode = lock_mode;
	newl->status = DB_LSTAT_HELD;
	newl->holder = locker;
	newl->refcount = 1;

	if ((ret =
	    __lock_getobj(lt, 0, (DBT *)obj, DB_LOCK_OBJTYPE, &sh_obj)) != 0)
		return (ret);

	lrp = lt->region;			/* getobj might have grown */
	newl = OFFSET_TO_LOCK(lt, newl_off);

	/* Now make new lock point to object */
	newl->obj = SH_PTR_TO_OFF(newl, sh_obj);

	/*
	 * Now we have a lock and an object and we need to see if we should
	 * grant the lock.  We use a FIFO ordering so we can only grant a
	 * new lock if it does not conflict with anyone on the holders list
	 * OR anyone on the waiters list.  The reason that we don't grant if
	 * there's a conflict is that this can lead to starvation (a writer
	 * waiting on a popularly read item will never ben granted).  The
	 * downside of this is that a waiting reader can prevent an upgrade
	 * from reader to writer, which is not uncommon.  In case of conflict,
	 * we put the new lock on the end of the waiters list.
	 */
	for (lp = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock);
	    lp != NULL;
	    lp = SH_TAILQ_NEXT(lp, links, __db_lock)) {
		if (CONFLICTS(lt, lp->mode, lock_mode) &&
		    locker != lp->holder)
			break;
		else if (lp->holder == locker && lp->mode == lock_mode &&
		    lp->status == DB_LSTAT_HELD) {
			/* Lock is already held, just inc the ref count. */
			lp->refcount++;
			SH_TAILQ_INSERT_HEAD(&lrp->free_locks, newl, links,
			    __db_lock);
			*lockp = lp;
			return (0);
		}
    	}

	if (lp == NULL)
		for (lp = SH_TAILQ_FIRST(&sh_obj->waiters, __db_lock);
		    lp != NULL;
		    lp = SH_TAILQ_NEXT(lp, links, __db_lock)) {
			if (CONFLICTS(lt, lp->mode, lock_mode) &&
			    locker != lp->holder)
				break;
		}
	if (lp == NULL)
		SH_TAILQ_INSERT_TAIL(&sh_obj->holders, newl, links);
	else if (!(flags & DB_LOCK_NOWAIT))
		SH_TAILQ_INSERT_TAIL(&sh_obj->waiters, newl, links);
	else {
		/* Free the lock and return an error. */
		newl->status = DB_LSTAT_FREE;
		SH_TAILQ_INSERT_HEAD(&lrp->free_locks, newl, links, __db_lock);
		return (DB_LOCK_NOTGRANTED);
	}

	/*
	 * This is really a blocker for the process, so initialize it
	 * set.  That way the current process will block when it tries
	 * to get it and the waking process will release it.
	 */
	(void)__db_mutex_init(&newl->mutex,
	    MUTEX_LOCK_OFFSET(lt->region, &newl->mutex));
	(void)__db_mutex_lock(&newl->mutex, lt->fd,
	    lt->dbenv == NULL ? NULL : lt->dbenv->db_yield);

	/*
	 * Now, insert the lock onto its locker's list.
	 */
	if ((ret =
	    __lock_getobj(lt, locker, NULL, DB_LOCK_LOCKER, &sh_locker)) != 0)
		return (ret);

	lrp = lt->region;
	SH_LIST_INSERT_HEAD(&sh_locker->heldby, newl, locker_links, __db_lock);

	if (lp != NULL) {
		newl->status = DB_LSTAT_WAITING;
		lrp->nconflicts++;
		/*
		 * We are about to wait; must release the region mutex.
		 * Then, when we wakeup, we need to reacquire the region
		 * mutex before continuing.
		 */
		if (lrp->detect == DB_LOCK_NORUN)
			lt->region->need_dd = 1;
		UNLOCK_LOCKREGION(lt);

		/*
		 * We are about to wait; before waiting, see if the deadlock
		 * detector should be run.
		 */
		if (lrp->detect != DB_LOCK_NORUN)
			ret = lock_detect(lt, 0, lrp->detect);

		(void)__db_mutex_lock(&newl->mutex,
		    lt->fd, lt->dbenv == NULL ? NULL : lt->dbenv->db_yield);

		LOCK_LOCKREGION(lt);
		if (newl->status != DB_LSTAT_PENDING) {
			/* Return to free list. */
			__lock_checklocker(lt, newl, 0);
			SH_TAILQ_INSERT_HEAD(&lrp->free_locks, newl, links,
			    __db_lock);
			switch (newl->status) {
				case DB_LSTAT_ABORTED:
					ret = DB_LOCK_DEADLOCK;
					break;
				case DB_LSTAT_NOGRANT:
					ret = DB_LOCK_NOTGRANTED;
					break;
				default:
					ret = EINVAL;
					break;
			}
			newl->status = DB_LSTAT_FREE;
			newl = NULL;
		} else
			newl->status = DB_LSTAT_HELD;
	}

	*lockp = newl;
	return (ret);
}

/*
 * This is called at every interface to verify if the region
 * has changed size, and if so, to remap the region in and
 * reset the process pointers.
 */
static int
__lock_validate_region(lt)
	DB_LOCKTAB *lt;
{
	int ret;

	if (lt->reg_size == lt->region->hdr.size)
		return (0);

	/* Grow the region. */
	if ((ret = __db_rremap(lt->dbenv, lt->region,
	    lt->reg_size, lt->region->hdr.size, lt->fd, &lt->region)) != 0)
		return (ret);

	__lock_reset_region(lt);

	return (0);
}

/*
 * We have run out of space; time to grow the region.
 */
static int
__lock_grow_region(lt, which, howmuch)
	DB_LOCKTAB *lt;
	int which;
	size_t howmuch;
{
	struct __db_lock *newl;
	struct lock_header *lock_head;
	struct obj_header *obj_head;
	DB_LOCKOBJ *op;
	DB_LOCKREGION *lrp;
	float lock_ratio, obj_ratio;
	size_t incr, oldsize, used;
	u_int32_t i, newlocks, newmem, newobjs;
	int ret, usedlocks, usedmem, usedobjs;
	u_int8_t *curaddr;

	lrp = lt->region;
	oldsize = lrp->hdr.size;
	incr = lrp->increment;

	/* Figure out how much of each sort of space we have. */
	usedmem = lrp->mem_bytes - __db_shalloc_count(lt->mem);
	usedobjs = lrp->numobjs - __lock_count_objs(lrp);
	usedlocks = lrp->maxlocks - __lock_count_locks(lrp);

	/*
	 * Figure out what fraction of the used space belongs to each
	 * different type of "thing" in the region.  Then partition the
	 * new space up according to this ratio.
	 */
	used = usedmem +
	    usedlocks * ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT) +
	    usedobjs * sizeof(DB_LOCKOBJ);

	lock_ratio = usedlocks *
	    ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT) / (float)used;
	obj_ratio = usedobjs * sizeof(DB_LOCKOBJ) / (float)used;

	newlocks = (u_int32_t)(lock_ratio *
	    incr / ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT));
	newobjs = (u_int32_t)(obj_ratio * incr / sizeof(DB_LOCKOBJ));
	newmem = incr -
	    (newobjs * sizeof(DB_LOCKOBJ) +
	    newlocks * ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT));

	/*
	 * Make sure we allocate enough memory for the object being
	 * requested.
	 */
	switch (which) {
		case DB_LOCK_LOCK:
			if (newlocks == 0) {
				newlocks = 10;
				incr += newlocks * sizeof(struct __db_lock);
			}
			break;
		case DB_LOCK_OBJ:
			if (newobjs == 0) {
				newobjs = 10;
				incr += newobjs * sizeof(DB_LOCKOBJ);
			}
			break;
		case DB_LOCK_MEM:
			if (newmem < howmuch * 2) {
				incr += howmuch * 2 - newmem;
				newmem = howmuch * 2;
			}
			break;
	}

	newmem += ALIGN(incr, sizeof(size_t)) - incr;
	incr = ALIGN(incr, sizeof(size_t));

	/*
	 * Since we are going to be allocating locks at the beginning of the
	 * new chunk, we need to make sure that the chunk is MUTEX_ALIGNMENT
	 * aligned.  We did not guarantee this when we created the region, so
	 * we may need to pad the old region by extra bytes to ensure this
	 * alignment.
	 */
	incr += ALIGN(oldsize, MUTEX_ALIGNMENT) - oldsize;

	__db_err(lt->dbenv,
	    "Growing lock region: %lu locks %lu objs %lu bytes",
	    (u_long)newlocks, (u_long)newobjs, (u_long)newmem);

	if ((ret = __db_rgrow(lt->dbenv, lt->fd, incr)) != 0)
		return (ret);
	if ((ret = __db_rremap(lt->dbenv,
	    lt->region, oldsize, oldsize + incr, lt->fd, &lt->region)) != 0)
		return (ret);
	__lock_reset_region(lt);

	/* Update region parameters. */
	lrp = lt->region;
	lrp->increment = incr << 1;
	lrp->maxlocks += newlocks;
	lrp->numobjs += newobjs;
	lrp->mem_bytes += newmem;

	curaddr = (u_int8_t *)lrp + oldsize;
	curaddr = (u_int8_t *)ALIGNP(curaddr, MUTEX_ALIGNMENT);

	/* Put new locks onto the free list. */
	lock_head = &lrp->free_locks;
	for (i = 0; i++ < newlocks;
	    curaddr += ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT)) {
		newl = (struct __db_lock *)curaddr;
		SH_TAILQ_INSERT_HEAD(lock_head, newl, links, __db_lock);
	}

	/* Put new objects onto the free list.  */
	obj_head = &lrp->free_objs;
	for (i = 0; i++ < newobjs; curaddr += sizeof(DB_LOCKOBJ)) {
		op = (DB_LOCKOBJ *)curaddr;
		SH_TAILQ_INSERT_HEAD(obj_head, op, links, __db_lockobj);
	}

	*((size_t *)curaddr) = newmem - sizeof(size_t);
	curaddr += sizeof(size_t);
	__db_shalloc_free(lt->mem, curaddr);

	return (0);
}

#ifdef DEBUG
void
__lock_dump_region(lt, flags)
	DB_LOCKTAB *lt;
	unsigned long flags;
{
	struct __db_lock *lp;
	DB_LOCKOBJ *op;
	DB_LOCKREGION *lrp;
	u_int32_t i, j;

	lrp = lt->region;

	printf("Lock region parameters\n");
	printf("%s:0x%x\t%s:%lu\t%s:%lu\t%s:%lu\n%s:%lu\t%s:%lu\t%s:%lu\t\n",
	    "magic      ", lrp->magic,
	    "version    ", (u_long)lrp->version,
	    "processes  ", (u_long)lrp->hdr.refcnt,
	    "maxlocks   ", (u_long)lrp->maxlocks,
	    "table size ", (u_long)lrp->table_size,
	    "nmodes     ", (u_long)lrp->nmodes,
	    "numobjs    ", (u_long)lrp->numobjs);
	printf("%s:%lu\t%s:%lu\t%s:%lu\n%s:%lu\t%s:%lu\t%s:%lu\n",
	    "size       ", (u_long)lrp->hdr.size,
	    "nlockers   ", (u_long)lrp->nlockers,
	    "hash_off   ", (u_long)lrp->hash_off,
	    "increment  ", (u_long)lrp->increment,
	    "mem_off    ", (u_long)lrp->mem_off,
	    "mem_bytes  ", (u_long)lrp->mem_bytes);
#ifndef HAVE_SPINLOCKS
	printf("Mutex: off %lu", (u_long)lrp->hdr.lock.off);
#endif
#ifdef MUTEX_STATISTICS
	printf(" waits %lu nowaits %lu",
	    (u_long)lrp->hdr.lock.mutex_set_wait,
	    (u_long)lrp->hdr.lock.mutex_set_nowait);
#endif
	printf("\n%s:%lu\t%s:%lu\t%s:%lu\t%s:%lu\n",
	    "nconflicts ", (u_long)lrp->nconflicts,
	    "nrequests  ", (u_long)lrp->nrequests,
	    "nreleases  ", (u_long)lrp->nreleases,
	    "ndeadlocks ", (u_long)lrp->ndeadlocks);
	printf("need_dd    %lu\n", (u_long)lrp->need_dd);
	if (flags & LOCK_DEBUG_CONF) {
		printf("\nConflict matrix\n");

		for (i = 0; i < lrp->nmodes; i++) {
			for (j = 0; j < lrp->nmodes; j++)
				printf("%lu\t",
				    (u_long)lt->conflicts[i * lrp->nmodes + j]);
			printf("\n");
		}
	}

	for (i = 0; i < lrp->table_size; i++) {
		op = SH_TAILQ_FIRST(&lt->hashtab[i], __db_lockobj);
		if (op != NULL && flags & LOCK_DEBUG_BUCKET)
			printf("Bucket %lu:\n", (unsigned long)i);
		while (op != NULL) {
			if (op->type == DB_LOCK_LOCKER &&
			    flags & LOCK_DEBUG_LOCKERS)
				__lock_dump_locker(lt, op);
			else if (flags & LOCK_DEBUG_OBJECTS &&
			    op->type == DB_LOCK_OBJTYPE)
				__lock_dump_object(lt, op);
			op = SH_TAILQ_NEXT(op, links, __db_lockobj);
		}
	}

	if (flags & LOCK_DEBUG_LOCK) {
		printf("\nLock Free List\n");
		for (lp = SH_TAILQ_FIRST(&lrp->free_locks, __db_lock);
		    lp != NULL;
		    lp = SH_TAILQ_NEXT(lp, links, __db_lock)) {
			printf("0x%x: %lu\t%lu\t%lu\t0x%x\n", (u_int)lp,
			    (u_long)lp->holder, (u_long)lp->mode,
			    (u_long)lp->status, (u_int)lp->obj);
		}
	}

	if (flags & LOCK_DEBUG_LOCK) {
		printf("\nObject Free List\n");
		for (op = SH_TAILQ_FIRST(&lrp->free_objs, __db_lockobj);
		    op != NULL;
		    op = SH_TAILQ_NEXT(op, links, __db_lockobj))
			printf("0x%x\n", (u_int)op);
	}

	if (flags & LOCK_DEBUG_MEM) {
		printf("\nMemory Free List\n");
		__db_shalloc_dump(stdout, lt->mem);
	}
}

static void
__lock_dump_locker(lt, op)
	DB_LOCKTAB *lt;
	DB_LOCKOBJ *op;
{
	struct __db_lock *lp;
	u_int32_t locker;
	void *ptr;

	ptr = SH_DBT_PTR(&op->lockobj);
	memcpy(&locker, ptr, sizeof(u_int32_t));
	printf("L %lx", (u_long)locker);

	lp = SH_LIST_FIRST(&op->heldby, __db_lock);
	if (lp == NULL) {
		printf("\n");
		return;
	}
	for (; lp != NULL; lp = SH_LIST_NEXT(lp, locker_links, __db_lock))
		__lock_printlock(lt, lp, 0);
}

static void
__lock_dump_object(lt, op)
	DB_LOCKTAB *lt;
	DB_LOCKOBJ *op;
{
	struct __db_lock *lp;
	u_int32_t j;
	char *ptr;

	ptr = SH_DBT_PTR(&op->lockobj);
	for (j = 0; j < op->lockobj.size; ptr++, j++)
		printf("%c", (int)*ptr);
	printf("\n");

	printf("H:");
	for (lp =
	    SH_TAILQ_FIRST(&op->holders, __db_lock);
	    lp != NULL;
	    lp = SH_TAILQ_NEXT(lp, links, __db_lock))
		__lock_printlock(lt, lp, 0);
	lp = SH_TAILQ_FIRST(&op->waiters, __db_lock);
	if (lp != NULL) {
		printf("\nW:");
		for (; lp != NULL; lp = SH_TAILQ_NEXT(lp, links, __db_lock))
			__lock_printlock(lt, lp, 0);
	}
}

int
__lock_is_locked(lt, locker, dbt, mode)
	DB_LOCKTAB *lt;
	u_int32_t locker;
	DBT *dbt;
	db_lockmode_t mode;
{
	struct __db_lock *lp;
	DB_LOCKOBJ *sh_obj;
	DB_LOCKREGION *lrp;

	lrp = lt->region;

	/* Look up the object in the hash table. */
	HASHLOOKUP(lt->hashtab, __db_lockobj, links,
	    dbt, sh_obj, lrp->table_size, __lock_ohash, __lock_cmp);
	if (sh_obj == NULL)
		return (0);

	for (lp = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock);
	    lp != NULL;
	    lp = SH_TAILQ_FIRST(&sh_obj->holders, __db_lock)) {
		if (lp->holder == locker && lp->mode == mode)
			return (1);
	}

	return (0);
}

static void
__lock_printlock(lt, lp, ispgno)
	DB_LOCKTAB *lt;
	struct __db_lock *lp;
	int ispgno;
{
	DB_LOCKOBJ *lockobj;
	db_pgno_t pgno;
	size_t obj;
	u_int8_t *ptr;
	char *mode, *stat;

	switch (lp->mode) {
	case DB_LOCK_IREAD:
		mode = "IREAD";
		break;
	case DB_LOCK_IWR:
		mode = "IWR";
		break;
	case DB_LOCK_IWRITE:
		mode = "IWRITE";
		break;
	case DB_LOCK_NG:
		mode = "NG";
		break;
	case DB_LOCK_READ:
		mode = "READ";
		break;
	case DB_LOCK_WRITE:
		mode = "WRITE";
		break;
	default:
		mode = "UNKNOWN";
		break;
	}
	switch (lp->status) {
	case DB_LSTAT_ABORTED:
		stat = "ABORT";
		break;
	case DB_LSTAT_ERR:
		stat = "ERROR";
		break;
	case DB_LSTAT_FREE:
		stat = "FREE";
		break;
	case DB_LSTAT_HELD:
		stat = "HELD";
		break;
	case DB_LSTAT_NOGRANT:
		stat = "NONE";
		break;
	case DB_LSTAT_WAITING:
		stat = "WAIT";
		break;
	case DB_LSTAT_PENDING:
		stat = "PENDING";
		break;
	default:
		stat = "UNKNOWN";
		break;
	}
	printf("\t%lx\t%s\t%lu\t%s\t",
	    (u_long)lp->holder, mode, (u_long)lp->refcount, stat);

	lockobj = (DB_LOCKOBJ *)((u_int8_t *)lp + lp->obj);
	ptr = SH_DBT_PTR(&lockobj->lockobj);
	if (ispgno) {
		/* Assume this is a DBT lock. */
		memcpy(&pgno, ptr, sizeof(db_pgno_t));
		printf("page %lu\n", (u_long)pgno);
	} else {
		obj = (u_int8_t *)lp + lp->obj - (u_int8_t *)lt->region;
		printf("0x%lx ", (u_long)obj);
		__db_pr(ptr, lockobj->lockobj.size);
		printf("\n");
	}
}

#endif

static int
__lock_count_locks(lrp)
	DB_LOCKREGION *lrp;
{
	struct __db_lock *newl;
	int count;

	count = 0;
	for (newl = SH_TAILQ_FIRST(&lrp->free_locks, __db_lock);
	    newl != NULL;
	    newl = SH_TAILQ_NEXT(newl, links, __db_lock))
		count++;

	return (count);
}

static int
__lock_count_objs(lrp)
	DB_LOCKREGION *lrp;
{
	DB_LOCKOBJ *obj;
	int count;

	count = 0;
	for (obj = SH_TAILQ_FIRST(&lrp->free_objs, __db_lockobj);
	    obj != NULL;
	    obj = SH_TAILQ_NEXT(obj, links, __db_lockobj))
		count++;

	return (count);
}

/*
 * PUBLIC: int __lock_getobj  __P((DB_LOCKTAB *,
 * PUBLIC:     u_int32_t, DBT *, u_int32_t type, DB_LOCKOBJ **));
 */
int
__lock_getobj(lt, locker, dbt, type, objp)
	DB_LOCKTAB *lt;
	u_int32_t locker, type;
	DBT *dbt;
	DB_LOCKOBJ **objp;
{
	DB_LOCKREGION *lrp;
	DB_LOCKOBJ *sh_obj;
	u_int32_t obj_size;
	int ret;
	void *p, *src;

	lrp = lt->region;

	/* Look up the object in the hash table. */
	if (type == DB_LOCK_OBJTYPE) {
		HASHLOOKUP(lt->hashtab, __db_lockobj, links, dbt, sh_obj,
		    lrp->table_size, __lock_ohash, __lock_cmp);
		obj_size = dbt->size;
	} else {
		HASHLOOKUP(lt->hashtab, __db_lockobj, links, locker,
		    sh_obj, lrp->table_size, __lock_locker_hash,
		    __lock_locker_cmp);
		obj_size = sizeof(locker);
	}

	/*
	 * If we found the object, then we can just return it.  If
	 * we didn't find the object, then we need to create it.
	 */
	if (sh_obj == NULL) {
		/* Create new object and then insert it into hash table. */
		if ((sh_obj = SH_TAILQ_FIRST(&lrp->free_objs, __db_lockobj))
		    == NULL) {
			if ((ret = __lock_grow_region(lt, DB_LOCK_OBJ, 0)) != 0)
				return (ret);
			lrp = lt->region;
			sh_obj = SH_TAILQ_FIRST(&lrp->free_objs, __db_lockobj);
		}
		if ((ret = __db_shalloc(lt->mem, obj_size, 0, &p)) != 0) {
			if ((ret = __lock_grow_region(lt,
			    DB_LOCK_MEM, obj_size)) != 0)
				return (ret);
			lrp = lt->region;
			/* Reacquire the head of the list. */
			sh_obj = SH_TAILQ_FIRST(&lrp->free_objs, __db_lockobj);
			(void)__db_shalloc(lt->mem, obj_size, 0, &p);
		}
		sh_obj->type = type;
		src = type == DB_LOCK_OBJTYPE ? dbt->data : (void *)&locker;
		memcpy(p, src, obj_size);
		SH_TAILQ_REMOVE(&lrp->free_objs, sh_obj, links, __db_lockobj);

		SH_TAILQ_INIT(&sh_obj->waiters);
		if (type == DB_LOCK_LOCKER)
			SH_LIST_INIT(&sh_obj->heldby);
		else
			SH_TAILQ_INIT(&sh_obj->holders);
		sh_obj->lockobj.size = obj_size;
		sh_obj->lockobj.off = SH_PTR_TO_OFF(&sh_obj->lockobj, p);

		HASHINSERT(lt->hashtab,
		    __db_lockobj, links, sh_obj, lrp->table_size, __lock_lhash);

		if (type == DB_LOCK_LOCKER)
			lrp->nlockers++;
	}

	*objp = sh_obj;
	return (0);
}

/*
 * Any lock on the waitlist has a process waiting for it.  Therefore, we
 * can't return the lock to the freelist immediately.  Instead, we can
 * remove the lock from the list of waiters, set the status field of the
 * lock, and then let the process waking up return the lock to the
 * free list.
 */
static void
__lock_remove_waiter(lt, sh_obj, lockp, status)
	DB_LOCKTAB *lt;
	DB_LOCKOBJ *sh_obj;
	struct __db_lock *lockp;
	db_status_t status;
{
	SH_TAILQ_REMOVE(&sh_obj->waiters, lockp, links, __db_lock);
	lockp->status = status;

	/* Wake whoever is waiting on this lock. */
	(void)__db_mutex_unlock(&lockp->mutex, lt->fd);
}

static void
__lock_freeobj(lt, obj)
	DB_LOCKTAB *lt;
	DB_LOCKOBJ *obj;
{
	HASHREMOVE_EL(lt->hashtab,
	    __db_lockobj, links, obj, lt->region->table_size, __lock_lhash);
	__db_shalloc_free(lt->mem, SH_DBT_PTR(&obj->lockobj));
	SH_TAILQ_INSERT_HEAD(&lt->region->free_objs, obj, links, __db_lockobj);
}

static void
__lock_checklocker(lt, lockp, do_remove)
	DB_LOCKTAB *lt;
	struct __db_lock *lockp;
	int do_remove;
{
	DB_LOCKOBJ *sh_locker;

	if (do_remove)
		SH_LIST_REMOVE(lockp, locker_links, __db_lock);

	/* if the locker list is NULL, free up the object. */
	if (__lock_getobj(lt, lockp->holder, NULL, DB_LOCK_LOCKER, &sh_locker)
	    == 0 && SH_LIST_FIRST(&sh_locker->heldby, __db_lock) == NULL) {
		__lock_freeobj(lt, sh_locker);
		lt->region->nlockers--;
	}
}

static void
__lock_reset_region(lt)
	DB_LOCKTAB *lt;
{
	lt->conflicts = (u_int8_t *)lt->region + sizeof(DB_LOCKREGION);
	lt->hashtab =
	    (DB_HASHTAB *)((u_int8_t *)lt->region + lt->region->hash_off);
	lt->mem = (void *)((u_int8_t *)lt->region + lt->region->mem_off);
	lt->reg_size = lt->region->hdr.size;
}
