/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)lock.h	10.9 (Sleepycat) 10/25/97
 */

typedef struct __db_lockobj	DB_LOCKOBJ;

#define DB_DEFAULT_LOCK_FILE	"__db_lock.share"
#define DB_LOCK_DEFAULT_N	5000

/*
 * The locker id space is divided between the transaction manager and the lock
 * manager.  Lockid's start at 0 and go to DB_LOCK_MAXID.  Txn Id's start at
 * DB_LOCK_MAXID + 1 and go up to TXN_INVALID.
 */
#define DB_LOCK_MAXID		0x7fffffff

/*
 * The lock region consists of:
 *	The DB_LOCKREGION structure (sizeof(DB_LOCKREGION)).
 *	The conflict matrix of nmodes * nmodes bytes (nmodes * nmodes).
 *	The hash table for object lookup (hashsize * sizeof(DB_OBJ *)).
 *	The locks themselves (maxlocks * sizeof(struct __db_lock).
 *	The objects being locked (maxlocks * sizeof(DB_OBJ)).
 *	String space to represent the DBTs that are the objects being locked.
 */
struct __db_lockregion {
	RLAYOUT		hdr;		/* Shared region header. */
	u_int32_t	magic;		/* lock magic number */
	u_int32_t	version;	/* version number */
	u_int32_t	id;		/* unique id generator */
	u_int32_t	need_dd;	/* flag for deadlock detector */
	u_int32_t	detect;		/* run dd on every conflict */
	SH_TAILQ_HEAD(lock_header) free_locks;	/* free lock header */
	SH_TAILQ_HEAD(obj_header) free_objs;	/* free obj header */
	u_int32_t	maxlocks;	/* maximum number of locks in table */
	u_int32_t	table_size;	/* size of hash table */
	u_int32_t	nmodes;		/* number of lock modes */
	u_int32_t	numobjs;	/* number of objects */
	u_int32_t	nlockers;	/* number of lockers */
	size_t		increment;	/* how much to grow region */
	size_t		hash_off;	/* offset of hash table */
	size_t		mem_off;	/* offset of memory region */
	size_t		mem_bytes;	/* number of bytes in memory region */
	u_int32_t	nconflicts;	/* number of lock conflicts */
	u_int32_t	nrequests;	/* number of lock gets */
	u_int32_t	nreleases;	/* number of lock puts */
	u_int32_t	ndeadlocks;	/* number of deadlocks */
};

/* Macros to lock/unlock the region. */
#define	LOCK_LOCKREGION(lt)						\
	(void)__db_mutex_lock(&(lt)->region->hdr.lock, (lt)->fd)
#define	UNLOCK_LOCKREGION(lt)						\
	(void)__db_mutex_unlock(&(lt)->region->hdr.lock, (lt)->fd)

/*
 * Since we will be keeping DBTs in shared memory, we need the equivalent
 * of a DBT that will work in shared memory.
 */
typedef struct __sh_dbt {
	u_int32_t size;
	ssize_t off;
} SH_DBT;

#define SH_DBT_PTR(p)	((void *)(((u_int8_t *)(p)) + (p)->off))

/*
 * The lock table is the per-process cookie returned from a lock_open call.
 */
struct __db_lockobj {
	SH_DBT	lockobj;		/* Identifies object locked. */
	SH_TAILQ_ENTRY links;		/* Links for free list. */
	union {
		SH_TAILQ_HEAD(_wait) _waiters;	/* List of waiting locks. */
		u_int32_t	_dd_id;		/* Deadlock detector id. */
	} wlinks;
	union {
		SH_LIST_HEAD(_held) _heldby;	/* Locks held by this locker. */
		SH_TAILQ_HEAD(_hold) _holders;	/* List of held locks. */
	} dlinks;
#define	DB_LOCK_OBJTYPE		1
#define	DB_LOCK_LOCKER		2
	u_int8_t type;			/* Real object or locker id. */
};


#define dd_id	wlinks._dd_id
#define	waiters	wlinks._waiters
#define	holders	dlinks._holders
#define	heldby	dlinks._heldby

struct __db_locktab {
	DB_ENV		*dbenv;		/* Environment. */
	int		 fd;		/* mapped file descriptor */
	DB_LOCKREGION	*region;	/* address of shared memory region */
	DB_HASHTAB 	*hashtab; 	/* Beginning of hash table. */
	size_t		reg_size;	/* last known size of lock region */
	void		*mem;		/* Beginning of string space. */
	u_int8_t 	*conflicts;	/* Pointer to conflict matrix. */
};

/* Test for conflicts. */
#define CONFLICTS(T, HELD, WANTED) \
	T->conflicts[HELD * T->region->nmodes + WANTED]

/*
 * Status of a lock.
 */
typedef enum {
	DB_LSTAT_ABORTED,		/* Lock belongs to an aborted txn. */
	DB_LSTAT_ERR,			/* Lock is bad. */
	DB_LSTAT_FREE,			/* Lock is unallocated. */
	DB_LSTAT_HELD,			/* Lock is currently held. */
	DB_LSTAT_NOGRANT,		/* Lock was not granted. */
	DB_LSTAT_PENDING,		/* Lock was waiting and has been
					 * promoted; waiting for the owner
					 * to run and upgrade it to held. */
	DB_LSTAT_WAITING		/* Lock is on the wait queue. */
} db_status_t;

/*
 * Resources in the lock region.  Used to indicate which resource
 * is running low when we need to grow the region.
 */
typedef enum {
	DB_LOCK_MEM, DB_LOCK_OBJ, DB_LOCK_LOCK
} db_resource_t;

struct __db_lock {
	/*
	 * Wait on mutex to wait on lock.  You reference your own mutex with
	 * ID 0 and others reference your mutex with ID 1.
	 */
	db_mutex_t	mutex;

	u_int32_t	holder;		/* Who holds this lock. */
	SH_TAILQ_ENTRY	links;		/* Free or holder/waiter list. */
	SH_LIST_ENTRY	locker_links;	/* List of locks held by a locker. */
	u_int32_t	refcount;	/* Reference count the lock. */
	db_lockmode_t	mode;		/* What sort of lock. */
	ssize_t		obj;		/* Relative offset of object struct. */
	db_status_t	status;		/* Status of this lock. */
};

/*
 * We cannot return pointers to the user (else we cannot easily grow regions),
 * so we return offsets in the region.  These must be converted to and from
 * regular pointers.  Always use the macros below.
 */
#define OFFSET_TO_LOCK(lt, off)	\
	((struct __db_lock *)((u_int8_t *)((lt)->region) + (off)))
#define LOCK_TO_OFFSET(lt, lock) \
	((size_t)((u_int8_t *)(lock) - (u_int8_t *)lt->region))
#define OFFSET_TO_OBJ(lt, off)	\
	((DB_LOCKOBJ *)((u_int8_t *)((lt)->region) + (off)))
#define OBJ_TO_OFFSET(lt, obj) \
	((size_t)((u_int8_t *)(obj) - (u_int8_t *)lt->region))

/*
 * The lock header contains the region structure and the conflict matrix.
 * Aligned to a large boundary because we don't know what the underlying
 * type of the hash table elements are.
 */
#define LOCK_HASH_ALIGN	8
#define LOCK_HEADER_SIZE(M)	\
	((size_t)(sizeof(DB_LOCKREGION) + ALIGN((M * M), LOCK_HASH_ALIGN)))

/*
 * For the full region, we need to add the locks, the objects, the hash table
 * and the string space (which is 16 bytes per lock).
 */
#define STRING_SIZE(N) (16 * N)

#define LOCK_REGION_SIZE(M, N, H)					\
	(ALIGN(LOCK_HEADER_SIZE(M) +					\
	(H) * sizeof(DB_HASHTAB), MUTEX_ALIGNMENT) +			\
	(N) * ALIGN(sizeof(struct __db_lock), MUTEX_ALIGNMENT) +	\
	ALIGN((N) * sizeof(DB_LOCKOBJ), sizeof(size_t)) +		\
	ALIGN(STRING_SIZE(N), sizeof(size_t)))

#ifdef DEBUG
#define	LOCK_DEBUG_LOCKERS	0x0001
#define	LOCK_DEBUG_LOCK	 	0x0002
#define	LOCK_DEBUG_OBJ	 	0x0004
#define	LOCK_DEBUG_CONF	 	0x0008
#define	LOCK_DEBUG_MEM	 	0x0010
#define	LOCK_DEBUG_BUCKET	0x0020
#define LOCK_DEBUG_OBJECTS	0x0040
#define	LOCK_DEBUG_ALL	 	0xFFFF

#define	LOCK_DEBUG_NOMUTEX	0x0100
#endif

#include "lock_ext.h"
