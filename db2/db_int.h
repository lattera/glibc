/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)db_int.h.src	10.41 (Sleepycat) 1/8/98
 */

#ifndef _DB_INTERNAL_H_
#define	_DB_INTERNAL_H_

#include "db.h"				/* Standard DB include file. */
#include "queue.h"
#include "os_func.h"
#include "os_ext.h"

/*******************************************************
 * General purpose constants and macros.
 *******************************************************/
#define	UINT16_T_MAX	    0xffff	/* Maximum 16 bit unsigned. */
#define	UINT32_T_MAX	0xffffffff	/* Maximum 32 bit unsigned. */

#define	DB_MIN_PGSIZE	0x000200	/* Minimum page size. */
#define	DB_MAX_PGSIZE	0x010000	/* Maximum page size. */

#define	DB_MINCACHE	10		/* Minimum cached pages */

#define	MEGABYTE	1048576

/*
 * If we are unable to determine the underlying filesystem block size, use
 * 8K on the grounds that most OS's use less than 8K as their VM page size.
 */
#define	DB_DEF_IOSIZE	(8 * 1024)

/*
 * Aligning items to particular sizes or in pages or memory.  ALIGNP is a
 * separate macro, as we've had to cast the pointer to different integral
 * types on different architectures.
 *
 * We cast pointers into unsigned longs when manipulating them because C89
 * guarantees that u_long is the largest available integral type and further,
 * to never generate overflows.  However, neither C89 or C9X  requires that
 * any integer type be large enough to hold a pointer, although C9X created
 * the intptr_t type, which is guaranteed to hold a pointer but may or may
 * not exist.  At some point in the future, we should test for intptr_t and
 * use it where available.
 */
#undef	ALIGNTYPE
#define	ALIGNTYPE		u_long
#undef	ALIGNP
#define	ALIGNP(value, bound)	ALIGN((ALIGNTYPE)value, bound)
#undef	ALIGN
#define	ALIGN(value, bound)	(((value) + (bound) - 1) & ~((bound) - 1))

/*
 * There are several on-page structures that are declared to have a number of
 * fields followed by a variable length array of items.  The structure size
 * without including the variable length array or the address of the first of
 * those elements can be found using SSZ.
 *
 * This macro can also be used to find the offset of a structure element in a
 * structure.  This is used in various places to copy structure elements from
 * unaligned memory references, e.g., pointers into a packed page.
 *
 * There are two versions because compilers object if you take the address of
 * an array.
 */
#undef	SSZ
#define SSZ(name, field)	((int)&(((name *)0)->field))

#undef	SSZA
#define SSZA(name, field)	((int)&(((name *)0)->field[0]))

/* Macros to return per-process address, offsets based on shared regions. */
#define	R_ADDR(base, offset)	((void *)((u_int8_t *)((base)->addr) + offset))
#define	R_OFFSET(base, p)	((u_int8_t *)(p) - (u_int8_t *)(base)->addr)

/* Free and free-string macros that overwrite memory during debugging. */
#ifdef DEBUG
#undef	FREE
#define	FREE(p, len) {							\
	memset(p, 0xff, len);						\
	__db_free(p);							\
}
#undef	FREES
#define	FREES(p) {							\
	FREE(p, strlen(p));						\
}
#else
#undef	FREE
#define	FREE(p, len) {							\
	__db_free(p);							\
}
#undef	FREES
#define	FREES(p) {							\
	__db_free(p);							\
}
#endif

/* Structure used to print flag values. */
typedef struct __fn {
	u_int32_t mask;			/* Flag value. */
	const char *name;		/* Flag name. */
} FN;

/* Set, clear and test flags. */
#define	F_SET(p, f)	(p)->flags |= (f)
#define	F_CLR(p, f)	(p)->flags &= ~(f)
#define	F_ISSET(p, f)	((p)->flags & (f))
#define	LF_SET(f)	(flags |= (f))
#define	LF_CLR(f)	(flags &= ~(f))
#define	LF_ISSET(f)	(flags & (f))

/* Display separator string. */
#undef	DB_LINE
#define	DB_LINE "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

/* Unused, or not-used-yet variable.  "Shut that bloody compiler up!" */
#define	COMPQUIET(n, v)	(n) = (v)

/*******************************************************
 * Files.
 *******************************************************/
#ifndef MAXPATHLEN		/* Maximum path length. */
#ifdef PATH_MAX
#define	MAXPATHLEN	PATH_MAX
#else
#define	MAXPATHLEN	1024
#endif
#endif

#define	PATH_DOT	"."	/* Current working directory. */
#define	PATH_SEPARATOR	"/"	/* Path separator character. */

#ifndef S_IRUSR			/* UNIX specific file permissions. */
#define	S_IRUSR	0000400		/* R for owner */
#define	S_IWUSR	0000200		/* W for owner */
#define	S_IRGRP	0000040		/* R for group */
#define	S_IWGRP	0000020		/* W for group */
#define	S_IROTH	0000004		/* R for other */
#define	S_IWOTH	0000002		/* W for other */
#endif

#ifndef S_ISDIR			/* UNIX specific: directory test. */
#define	S_ISDIR(m)	((m & 0170000) == 0040000)
#endif

/*******************************************************
 * Mutex support.
 *******************************************************/
typedef unsigned char tsl_t;



/*
 * !!!
 * Various systems require different alignments for mutexes (the worst we've
 * seen so far is 16-bytes on some HP architectures).  The mutex (tsl_t) must
 * be first in the db_mutex_t structure, which must itself be first in the
 * region.  This ensures the alignment is as returned by mmap(2), which should
 * be sufficient.  All other mutex users must ensure proper alignment locally.
 */
#define	MUTEX_ALIGNMENT	1

/*
 * The offset of a mutex in memory.
 *
 * !!!
 * Not an off_t, so backing file offsets MUST be less than 4Gb.  See the
 * off field of the db_mutex_t as well.
 */
#define	MUTEX_LOCK_OFFSET(a, b)	((u_int32_t)((u_int8_t *)b - (u_int8_t *)a))

typedef struct _db_mutex_t {
#ifdef HAVE_SPINLOCKS
	tsl_t	  tsl_resource;		/* Resource test and set. */
#ifdef DEBUG
	u_long	  pid;			/* Lock holder: 0 or process pid. */
#endif
#else
	u_int32_t off;			/* Backing file offset. */
	u_long	  pid;			/* Lock holder: 0 or process pid. */
#endif
	u_int32_t spins;		/* Spins before block. */
	u_int32_t mutex_set_wait;	/* Granted after wait. */
	u_int32_t mutex_set_nowait;	/* Granted without waiting. */
} db_mutex_t;

#include "mutex_ext.h"

/*******************************************************
 * Access methods.
 *******************************************************/
/* Lock/unlock a DB thread. */
#define	DB_THREAD_LOCK(dbp)						\
	(F_ISSET(dbp, DB_AM_THREAD) ?					\
	    __db_mutex_lock((db_mutex_t *)(dbp)->mutexp, -1) : 0)
#define	DB_THREAD_UNLOCK(dbp)						\
	(F_ISSET(dbp, DB_AM_THREAD) ?					\
	    __db_mutex_unlock((db_mutex_t *)(dbp)->mutexp, -1) : 0)

/* Btree/recno local statistics structure. */
struct __db_bt_lstat;	typedef struct __db_bt_lstat DB_BTREE_LSTAT;
struct __db_bt_lstat {
	u_int32_t bt_freed;		/* Pages freed for reuse. */
	u_int32_t bt_pfxsaved;		/* Bytes saved by prefix compression. */
	u_int32_t bt_split;		/* Total number of splits. */
	u_int32_t bt_rootsplit;		/* Root page splits. */
	u_int32_t bt_fastsplit;		/* Fast splits. */
	u_int32_t bt_added;		/* Items added. */
	u_int32_t bt_deleted;		/* Items deleted. */
	u_int32_t bt_get;		/* Items retrieved. */
	u_int32_t bt_cache_hit;		/* Hits in fast-insert code. */
	u_int32_t bt_cache_miss;	/* Misses in fast-insert code. */
};

/*******************************************************
 * Environment.
 *******************************************************/
/* Type passed to __db_appname(). */
typedef enum {
	DB_APP_NONE=0,			/* No type (region). */
	DB_APP_DATA,			/* Data file. */
	DB_APP_LOG,			/* Log file. */
	DB_APP_TMP			/* Temporary file. */
} APPNAME;

/*******************************************************
 * Regions.
 *******************************************************/
/*
 * The shared memory regions share an initial structure so that the general
 * region code can handle races between the region being deleted and other
 * processes waiting on the region mutex.
 *
 * !!!
 * Note, the mutex must be the first entry in the region; see comment above.
 */
typedef struct _rlayout {
	db_mutex_t lock;		/* Region mutex. */
	u_int32_t  refcnt;		/* Region reference count. */
	size_t	   size;		/* Region length. */
	int	   majver;		/* Major version number. */
	int	   minver;		/* Minor version number. */
	int	   patch;		/* Patch version number. */

#define	DB_R_DELETED	0x01		/* Region was deleted. */
	u_int32_t  flags;
} RLAYOUT;

/*******************************************************
 * Mpool.
 *******************************************************/
/*
 * File types for DB access methods.  Negative numbers are reserved to DB.
 */
#define	DB_FTYPE_BTREE		-1	/* Btree. */
#define	DB_FTYPE_HASH		-2	/* Hash. */

/* Structure used as the DB pgin/pgout pgcookie. */
typedef struct __dbpginfo {
	size_t	db_pagesize;		/* Underlying page size. */
	int	needswap;		/* If swapping required. */
} DB_PGINFO;

/*******************************************************
 * Log.
 *******************************************************/
/* Initialize an LSN to 'zero'. */
#define	ZERO_LSN(LSN) {							\
	(LSN).file = 0;							\
	(LSN).offset = 0;						\
}

/* Return 1 if LSN is a 'zero' lsn, otherwise return 0. */
#define	IS_ZERO_LSN(LSN)	((LSN).file == 0)

/* Test if we need to log a change. */
#define	DB_LOGGING(dbp)							\
	(F_ISSET(dbp, DB_AM_LOGGING) && !F_ISSET(dbp, DB_AM_RECOVER))

#ifdef DEBUG
/*
 * Debugging macro to log operations.
 *	If DEBUG_WOP is defined, log operations that modify the database.
 *	If DEBUG_ROP is defined, log operations that read the database.
 *
 * D dbp
 * T txn
 * O operation (string)
 * K key
 * A data
 * F flags
 */
#define	LOG_OP(D, T, O, K, A, F) {					\
	DB_LSN _lsn;							\
	DBT _op;							\
	if (DB_LOGGING((D))) {						\
		memset(&_op, 0, sizeof(_op));				\
		_op.data = O;						\
		_op.size = strlen(O) + 1;				\
		(void)__db_debug_log((D)->dbenv->lg_info,		\
		    T, &_lsn, 0, &_op, (D)->log_fileid, K, A, F);	\
	}								\
}
#ifdef DEBUG_ROP
#define	DEBUG_LREAD(D, T, O, K, A, F)	LOG_OP(D, T, O, K, A, F)
#else
#define	DEBUG_LREAD(D, T, O, K, A, F)
#endif
#ifdef DEBUG_WOP
#define	DEBUG_LWRITE(D, T, O, K, A, F)	LOG_OP(D, T, O, K, A, F)
#else
#define	DEBUG_LWRITE(D, T, O, K, A, F)
#endif
#else
#define	DEBUG_LREAD(D, T, O, K, A, F)
#define	DEBUG_LWRITE(D, T, O, K, A, F)
#endif /* DEBUG */

/*******************************************************
 * Transactions and recovery.
 *******************************************************/
/*
 * Out of band value for a lock.  The locks are returned to callers as offsets
 * into the lock regions.  Since the RLAYOUT structure begins all regions, an
 * offset of 0 is guaranteed not to be a valid lock.
 */
#define	LOCK_INVALID	0

/* The structure allocated for every transaction. */
struct __db_txn {
	DB_TXNMGR	*mgrp;		/* Pointer to transaction manager. */
	DB_TXN		*parent;	/* Pointer to transaction's parent. */
	DB_LSN		last_lsn;	/* Lsn of last log write. */
	u_int32_t	txnid;		/* Unique transaction id. */
	size_t		off;		/* Detail structure within region. */
	TAILQ_ENTRY(__db_txn) links;
};
#endif /* !_DB_INTERNAL_H_ */
