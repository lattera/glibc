/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)db.h	10.174 (Sleepycat) 1/3/99
 */

#ifndef _DB_H_
#define	_DB_H_

#ifndef __NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <stdio.h>
#endif

/*
 * XXX
 * MacOS: ensure that Metrowerks C makes enumeration types int sized.
 */
#ifdef __MWERKS__
#pragma enumsalwaysint on
#endif

/*
 * XXX
 * Handle function prototypes and the keyword "const".  This steps on name
 * space that DB doesn't control, but all of the other solutions are worse.
 *
 * XXX
 * While Microsoft's compiler is ANSI C compliant, it doesn't have _STDC_
 * defined by default, you specify a command line flag or #pragma to turn
 * it on.  Don't do that, however, because some of Microsoft's own header
 * files won't compile.
 */
#undef	__P
#if defined(__STDC__) || defined(__cplusplus) || defined(_MSC_VER)
#define	__P(protos)	protos		/* ANSI C prototypes */
#else
#define	const
#define	__P(protos)	()		/* K&R C preprocessor */
#endif

/*
 * !!!
 * DB needs basic information about specifically sized types.  If they're
 * not provided by the system, typedef them here.
 *
 * We protect them against multiple inclusion using __BIT_TYPES_DEFINED__,
 * as does BIND and Kerberos, since we don't know for sure what #include
 * files the user is using.
 *
 * !!!
 * We also provide the standard u_int, u_long etc., if they're not provided
 * by the system.
 */

#define	DB_VERSION_MAJOR	2
#define	DB_VERSION_MINOR	7
#define	DB_VERSION_PATCH	5
#define	DB_VERSION_STRING	"Sleepycat Software: Berkeley DB 2.7.5: (04/18/99)"

typedef	u_int32_t	db_pgno_t;	/* Page number type. */
typedef	u_int16_t	db_indx_t;	/* Page offset type. */
#define	DB_MAX_PAGES	0xffffffff	/* >= # of pages in a file */

typedef	u_int32_t	db_recno_t;	/* Record number type. */
#define	DB_MAX_RECORDS	0xffffffff	/* >= # of records in a tree */

typedef size_t		DB_LOCK;	/* Object returned by lock manager. */

/* Forward structure declarations, so applications get type checking. */
struct __db;		typedef struct __db DB;
#ifdef DB_DBM_HSEARCH
			typedef struct __db DBM;
#endif
struct __db_bt_stat;	typedef struct __db_bt_stat DB_BTREE_STAT;
struct __db_dbt;	typedef struct __db_dbt DBT;
struct __db_env;	typedef struct __db_env DB_ENV;
struct __db_ilock;	typedef struct __db_ilock DB_LOCK_ILOCK;
struct __db_info;	typedef struct __db_info DB_INFO;
struct __db_lock_stat;	typedef struct __db_lock_stat DB_LOCK_STAT;
struct __db_lockregion;	typedef struct __db_lockregion DB_LOCKREGION;
struct __db_lockreq;	typedef struct __db_lockreq DB_LOCKREQ;
struct __db_locktab;	typedef struct __db_locktab DB_LOCKTAB;
struct __db_log;	typedef struct __db_log DB_LOG;
struct __db_log_stat;	typedef struct __db_log_stat DB_LOG_STAT;
struct __db_lsn;	typedef struct __db_lsn DB_LSN;
struct __db_mpool;	typedef struct __db_mpool DB_MPOOL;
struct __db_mpool_finfo;typedef struct __db_mpool_finfo DB_MPOOL_FINFO;
struct __db_mpool_fstat;typedef struct __db_mpool_fstat DB_MPOOL_FSTAT;
struct __db_mpool_stat;	typedef struct __db_mpool_stat DB_MPOOL_STAT;
struct __db_mpoolfile;	typedef struct __db_mpoolfile DB_MPOOLFILE;
struct __db_txn;	typedef struct __db_txn DB_TXN;
struct __db_txn_active;	typedef struct __db_txn_active DB_TXN_ACTIVE;
struct __db_txn_stat;	typedef struct __db_txn_stat DB_TXN_STAT;
struct __db_txnmgr;	typedef struct __db_txnmgr DB_TXNMGR;
struct __db_txnregion;	typedef struct __db_txnregion DB_TXNREGION;
struct __dbc;		typedef struct __dbc DBC;

/* Key/data structure -- a Data-Base Thang. */
struct __db_dbt {
	void	 *data;			/* key/data */
	u_int32_t size;			/* key/data length */
	u_int32_t ulen;			/* RO: length of user buffer. */
	u_int32_t dlen;			/* RO: get/put record length. */
	u_int32_t doff;			/* RO: get/put record offset. */

#define	DB_DBT_INTERNAL	0x01		/* Ignore user's malloc (internal). */
#define	DB_DBT_MALLOC	0x02		/* Return in allocated memory. */
#define	DB_DBT_PARTIAL	0x04		/* Partial put/get. */
#define	DB_DBT_USERMEM	0x08		/* Return in user's memory. */
	u_int32_t flags;
};

/*
 * DB run-time interface configuration.
 *
 * There are a set of functions that the application can replace with its
 * own versions, and some other knobs which can be turned at run-time.
 */
#define	DB_FUNC_CLOSE	 1		/* POSIX 1003.1 close. */
#define	DB_FUNC_DIRFREE	 2		/* DB: free directory list. */
#define	DB_FUNC_DIRLIST	 3		/* DB: create directory list. */
#define	DB_FUNC_EXISTS	 4		/* DB: return if file exists. */
#define	DB_FUNC_FREE	 5		/* ANSI C free. */
#define	DB_FUNC_FSYNC	 6		/* POSIX 1003.1 fsync. */
#define	DB_FUNC_IOINFO	 7		/* DB: return file I/O information. */
#define	DB_FUNC_MALLOC	 8		/* ANSI C malloc. */
#define	DB_FUNC_MAP	 9		/* DB: map file into shared memory. */
#define	DB_FUNC_OPEN	10		/* POSIX 1003.1 open. */
#define	DB_FUNC_READ	11		/* POSIX 1003.1 read. */
#define	DB_FUNC_REALLOC	12		/* ANSI C realloc. */
#define	DB_FUNC_RUNLINK	13		/* DB: remove a shared region. */
#define	DB_FUNC_SEEK	14		/* POSIX 1003.1 lseek. */
#define	DB_FUNC_SLEEP	15		/* DB: sleep secs/usecs. */
#define	DB_FUNC_UNLINK	16		/* POSIX 1003.1 unlink. */
#define	DB_FUNC_UNMAP	17		/* DB: unmap shared memory file. */
#define	DB_FUNC_WRITE	18		/* POSIX 1003.1 write. */
#define	DB_FUNC_YIELD	19		/* DB: yield thread to scheduler. */
#define	DB_MUTEXLOCKS	20		/* DB: turn off all mutex locks. */
#define	DB_PAGEYIELD	21		/* DB: yield the CPU on pool get. */
#define	DB_REGION_ANON	22		/* DB: anonymous, unnamed regions. */
#define	DB_REGION_INIT	23		/* DB: page-fault regions in create. */
#define	DB_REGION_NAME	24		/* DB: anonymous, named regions. */
#define	DB_TSL_SPINS	25		/* DB: initialize spin count. */

/*
 * Database configuration and initialization.
 */
 /*
  * Flags understood by both db_open(3) and db_appinit(3).
  */
#define	DB_CREATE	      0x000001	/* O_CREAT: create file as necessary. */
#define	DB_NOMMAP	      0x000002	/* Don't mmap underlying file. */
#define	DB_THREAD	      0x000004	/* Free-thread DB package handles. */

/*
 * Flags understood by db_appinit(3).
 */
/*			      0x000007	   COMMON MASK. */
#define	DB_INIT_CDB	      0x000008	/* Concurrent Access Methods. */
#define	DB_INIT_LOCK	      0x000010	/* Initialize locking. */
#define	DB_INIT_LOG	      0x000020	/* Initialize logging. */
#define	DB_INIT_MPOOL	      0x000040	/* Initialize mpool. */
#define	DB_INIT_TXN	      0x000080	/* Initialize transactions. */
#define	DB_MPOOL_PRIVATE      0x000100	/* Mpool: private memory pool. */
#define	DB_RECOVER	      0x000200	/* Run normal recovery. */
#define	DB_RECOVER_FATAL      0x000400	/* Run catastrophic recovery. */
#define	DB_TXN_NOSYNC	      0x000800	/* Do not sync log on commit. */
#define	DB_USE_ENVIRON	      0x001000	/* Use the environment. */
#define	DB_USE_ENVIRON_ROOT   0x002000	/* Use the environment if root. */

/*
 * Flags understood by db_open(3).
 *
 * DB_EXCL and DB_TEMPORARY are internal only, and are not documented.
 * DB_SEQUENTIAL is currently internal, but may be exported some day.
 */
/*			      0x000007	   COMMON MASK. */
/*			      0x001fff	   ALREADY USED. */
#define	DB_EXCL		      0x002000	/* O_EXCL: exclusive open (internal). */
#define	DB_RDONLY	      0x004000	/* O_RDONLY: read-only. */
#define	DB_SEQUENTIAL	      0x008000	/* Sequential access (internal). */
#define	DB_TEMPORARY	      0x010000	/* Remove on last close (internal). */
#define	DB_TRUNCATE	      0x020000	/* O_TRUNCATE: replace existing DB. */

/*
 * Deadlock detector modes; used in the DBENV structure to configure the
 * locking subsystem.
 */
#define	DB_LOCK_NORUN		0
#define	DB_LOCK_DEFAULT		1	/* Default policy. */
#define	DB_LOCK_OLDEST		2	/* Abort oldest transaction. */
#define	DB_LOCK_RANDOM		3	/* Abort random transaction. */
#define	DB_LOCK_YOUNGEST	4	/* Abort youngest transaction. */

struct __db_env {
	int		 db_lorder;	/* Byte order. */

					/* Error message callback. */
	void (*db_errcall) (const char *, char *);
	FILE		*db_errfile;	/* Error message file stream. */
	const char	*db_errpfx;	/* Error message prefix. */
	int		 db_verbose;	/* Generate debugging messages. */
	int		 db_panic;	/* Panic flag, callback function. */
	void (*db_paniccall) (DB_ENV *, int);

	/* User paths. */
	char		*db_home;	/* Database home. */
	char		*db_log_dir;	/* Database log file directory. */
	char		*db_tmp_dir;	/* Database tmp file directory. */

	char	       **db_data_dir;	/* Database data file directories. */
	int		 data_cnt;	/* Database data file slots. */
	int		 data_next;	/* Next Database data file slot. */

	/* Locking. */
	DB_LOCKTAB	*lk_info;	/* Return from lock_open(). */
	const u_int8_t	*lk_conflicts;	/* Two dimensional conflict matrix. */
	u_int32_t	 lk_modes;	/* Number of lock modes in table. */
	u_int32_t	 lk_max;	/* Maximum number of locks. */
	u_int32_t	 lk_detect;	/* Deadlock detect on all conflicts. */

	/* Logging. */
	DB_LOG		*lg_info;	/* Return from log_open(). */
	u_int32_t	 lg_max;	/* Maximum file size. */

	/* Memory pool. */
	DB_MPOOL	*mp_info;	/* Return from memp_open(). */
	size_t		 mp_mmapsize;	/* Maximum file size for mmap. */
	size_t		 mp_size;	/* Bytes in the mpool cache. */

	/* Transactions. */
	DB_TXNMGR	*tx_info;	/* Return from txn_open(). */
	u_int32_t	 tx_max;	/* Maximum number of transactions. */
	int (*tx_recover)		/* Dispatch function for recovery. */
	    (DB_LOG *, DBT *, DB_LSN *, int, void *);

	/*
	 * XA support.
	 *
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__db_env);
	 */
	struct {
		struct __db_env *tqe_next;
		struct __db_env **tqe_prev;
	} links;
	int		 xa_rmid;	/* XA Resource Manager ID. */
	DB_TXN		*xa_txn;	/* XA Current transaction. */

#define	DB_ENV_APPINIT		0x01	/* Paths initialized by db_appinit(). */
#define	DB_ENV_CDB		0x02	/* Concurrent DB product. */
#define	DB_ENV_STANDALONE	0x04	/* Test: freestanding environment. */
#define	DB_ENV_THREAD		0x08	/* DB_ENV is multi-threaded. */
	u_int32_t	 flags;		/* Flags. */
};

/*******************************************************
 * Access methods.
 *******************************************************/
/*
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_BTREE=1,			/* B+tree. */
	DB_HASH,			/* Extended Linear Hashing. */
	DB_RECNO,			/* Fixed and variable-length records. */
	DB_UNKNOWN			/* Figure it out on open. */
} DBTYPE;

#define	DB_BTREEVERSION	6		/* Current btree version. */
#define	DB_BTREEOLDVER	6		/* Oldest btree version supported. */
#define	DB_BTREEMAGIC	0x053162

#define	DB_HASHVERSION	5		/* Current hash version. */
#define	DB_HASHOLDVER	4		/* Oldest hash version supported. */
#define	DB_HASHMAGIC	0x061561

#define	DB_LOGVERSION	2		/* Current log version. */
#define	DB_LOGOLDVER	2		/* Oldest log version supported. */
#define	DB_LOGMAGIC	0x040988

struct __db_info {
	int		 db_lorder;	/* Byte order. */
	size_t		 db_cachesize;	/* Underlying cache size. */
	size_t		 db_pagesize;	/* Underlying page size. */

					/* Local heap allocation. */
	void *(*db_malloc) (size_t);
	int (*dup_compare)		/* Duplicate compare function. */
	    (const DBT *, const DBT *);

	/* Btree access method. */
	u_int32_t	 bt_maxkey;	/* Maximum keys per page. */
	u_int32_t	 bt_minkey;	/* Minimum keys per page. */
	int (*bt_compare)		/* Comparison function. */
	    (const DBT *, const DBT *);
	size_t (*bt_prefix)		/* Prefix function. */
	    (const DBT *, const DBT *);

	/* Hash access method. */
	u_int32_t 	 h_ffactor;	/* Fill factor. */
	u_int32_t	 h_nelem;	/* Number of elements. */
	u_int32_t      (*h_hash)	/* Hash function. */
	    (const void *, u_int32_t);

	/* Recno access method. */
	int		 re_pad;	/* Fixed-length padding byte. */
	int		 re_delim;	/* Variable-length delimiting byte. */
	u_int32_t	 re_len;	/* Length for fixed-length records. */
	char		*re_source;	/* Source file name. */

#define	DB_DELIMITER		0x0001	/* Recno: re_delim set. */
#define	DB_DUP			0x0002	/* Btree, Hash: duplicate keys. */
#define	DB_DUPSORT		0x0004	/* Btree, Hash: duplicate keys. */
#define	DB_FIXEDLEN		0x0008	/* Recno: fixed-length records. */
#define	DB_PAD			0x0010	/* Recno: re_pad set. */
#define	DB_RECNUM		0x0020	/* Btree: record numbers. */
#define	DB_RENUMBER		0x0040	/* Recno: renumber on insert/delete. */
#define	DB_SNAPSHOT		0x0080	/* Recno: snapshot the input. */
	u_int32_t	 flags;
};

/*
 * DB access method and cursor operation values.  Each value is an operation
 * code to which additional bit flags are added.
 */
#define	DB_AFTER	 1		/* c_put() */
#define	DB_APPEND	 2		/* put() */
#define	DB_BEFORE	 3		/* c_put() */
#define	DB_CHECKPOINT	 4		/* log_put(), log_get() */
#define	DB_CURLSN	 5		/* log_put() */
#define	DB_CURRENT	 6		/* c_get(), c_put(), log_get() */
#define	DB_FIRST	 7		/* c_get(), log_get() */
#define	DB_FLUSH	 8		/* log_put() */
#define	DB_GET_BOTH	 9		/* get(), c_get() */
#define	DB_GET_RECNO	10		/* c_get() */
#define	DB_JOIN_ITEM	11		/* c_get(); do not do primary lookup */
#define	DB_KEYFIRST	12		/* c_put() */
#define	DB_KEYLAST	13		/* c_put() */
#define	DB_LAST		14		/* c_get(), log_get() */
#define	DB_NEXT		15		/* c_get(), log_get() */
#define	DB_NEXT_DUP	16		/* c_get() */
#define	DB_NOOVERWRITE	17		/* put() */
#define	DB_NOSYNC	18		/* close() */
#define	DB_PREV		19		/* c_get(), log_get() */
#define	DB_RECORDCOUNT	20		/* stat() */
#define	DB_SET		21		/* c_get(), log_get() */
#define	DB_SET_RANGE	22		/* c_get() */
#define	DB_SET_RECNO	23		/* get(), c_get() */
#define	DB_WRITELOCK	24		/* cursor() (internal) */

#define	DB_OPFLAGS_MASK	0x1f		/* Mask for operations flags. */
#define	DB_RMW		0x80000000	/* Acquire write flag immediately. */

/*
 * DB (user visible) error return codes.
 *
 * !!!
 * Changes to any of the user visible error return codes must be reflected
 * in java/src/com/sleepycat/db/Db.java.
 */
#define	DB_INCOMPLETE		( -1)	/* Sync didn't finish. */
#define	DB_KEYEMPTY		( -2)	/* The key/data pair was deleted or
					   was never created by the user. */
#define	DB_KEYEXIST		( -3)	/* The key/data pair already exists. */
#define	DB_LOCK_DEADLOCK	( -4)	/* Locker killed to resolve deadlock. */
#define	DB_LOCK_NOTGRANTED	( -5)	/* Lock unavailable, no-wait set. */
#define	DB_LOCK_NOTHELD		( -6)	/* Lock not held by locker. */
#define	DB_NOTFOUND		( -7)	/* Key/data pair not found (EOF). */
#define	DB_RUNRECOVERY		( -8)	/* Panic return. */

/* DB (private) error return codes. */
#define	DB_DELETED		( -9)	/* Recovery file marked deleted. */
#define	DB_NEEDSPLIT		(-10)	/* Page needs to be split. */
#define	DB_SWAPBYTES		(-11)	/* Database needs byte swapping. */
#define	DB_TXN_CKP		(-12)	/* Encountered ckp record in log. */

#define	DB_FILE_ID_LEN		20	/* DB file ID length. */

/* DB access method description structure. */
struct __db {
	void	*mutexp;		/* Synchronization for free threading */

					/* Documented, returned information. */
	DBTYPE	 type;			/* DB access method. */
	int	 byteswapped;		/* Database byte order is swapped. */

	DB_ENV	*dbenv;			/* DB_ENV structure. */
	DB_ENV	*mp_dbenv;		/* DB_ENV for local mpool creation. */

	void	*internal;		/* Access method private. */

	DB_MPOOL	*mp;		/* The access method's mpool. */
	DB_MPOOLFILE	*mpf;		/* The access method's mpool file. */

	/*
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_HEAD(free_queue, __dbc);
	 * TAILQ_HEAD(active_queue, __dbc);
	 */
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} free_queue;
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} active_queue;

	u_int8_t  fileid[DB_FILE_ID_LEN]; /* Uniquely identify this file for
					     locking. */
	u_int32_t log_fileid;		/* Logging file id. */
	size_t	  pgsize;		/* Logical page size of file. */

					/* Local heap allocation. */
	void *(*db_malloc) (size_t);
	int (*dup_compare)		/* Duplicate compare function. */
	    (const DBT *, const DBT *);
	u_int32_t (*h_hash)		/* Hash function. */
	    (const void *, u_int32_t);

					/* Functions. */
	int (*am_close)	(DB *);
	int (*close)	(DB *, u_int32_t);
	int (*cursor)	(DB *, DB_TXN *, DBC **, u_int32_t);
	int (*del)	(DB *, DB_TXN *, DBT *, u_int32_t);
	int (*fd)	(DB *, int *);
	int (*get)	(DB *, DB_TXN *, DBT *, DBT *, u_int32_t);
	int (*join)	(DB *, DBC **, u_int32_t, DBC **);
	int (*put)	(DB *, DB_TXN *, DBT *, DBT *, u_int32_t);
	int (*stat)	(DB *, void *, void *(*)(size_t), u_int32_t);
	int (*sync)	(DB *, u_int32_t);

#define	DB_AM_CDB	0x000001	/* Concurrent Access Methods. */
#define	DB_AM_DUP	0x000002	/* DB_DUP (internal). */
#define	DB_AM_INMEM	0x000004	/* In-memory; no sync on close. */
#define	DB_AM_LOCKING	0x000008	/* Perform locking. */
#define	DB_AM_LOGGING	0x000010	/* Perform logging. */
#define	DB_AM_MLOCAL	0x000020	/* Database memory pool is local. */
#define	DB_AM_PGDEF	0x000040	/* Page size was defaulted. */
#define	DB_AM_RDONLY	0x000080	/* Database is readonly. */
#define	DB_AM_SWAP	0x000100	/* Pages need to be byte-swapped. */
#define	DB_AM_THREAD	0x000200	/* DB is multi-threaded. */
#define	DB_BT_RECNUM	0x000400	/* DB_RECNUM (internal). */
#define	DB_DBM_ERROR	0x000800	/* Error in DBM/NDBM database. */
#define	DB_RE_DELIMITER	0x001000	/* DB_DELIMITER (internal). */
#define	DB_RE_FIXEDLEN	0x002000	/* DB_FIXEDLEN (internal). */
#define	DB_RE_PAD	0x004000	/* DB_PAD (internal). */
#define	DB_RE_RENUMBER	0x008000	/* DB_RENUMBER (internal). */
#define	DB_RE_SNAPSHOT	0x010000	/* DB_SNAPSHOT (internal). */
	u_int32_t flags;
};

struct __db_ilock {			/* Internal DB access method lock. */
	db_pgno_t pgno;			/* Page being locked. */
	u_int8_t fileid[DB_FILE_ID_LEN];/* File id. */
};

/* Cursor description structure. */
struct __dbc {
	DB *dbp;			/* Related DB access method. */
	DB_TXN	 *txn;			/* Associated transaction. */

	/*
	 * !!!
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__dbc);
	 */
	struct {
		struct __dbc *tqe_next;
		struct __dbc **tqe_prev;
	} links;

	u_int32_t lid;			/* Default process' locker id. */
	u_int32_t locker;		/* Locker for this operation. */
	DBT	  lock_dbt;		/* DBT referencing lock. */
	DB_LOCK_ILOCK lock;		/* Object to be locked. */
	DB_LOCK	mylock;			/* Lock held on this cursor. */

	DBT rkey;			/* Returned key. */
	DBT rdata;			/* Returned data. */

	int (*c_am_close) (DBC *);
	int (*c_am_destroy) (DBC *);
	int (*c_close) (DBC *);
	int (*c_del) (DBC *, u_int32_t);
	int (*c_get) (DBC *, DBT *, DBT *, u_int32_t);
	int (*c_put) (DBC *, DBT *, DBT *, u_int32_t);

	void	 *internal;		/* Access method private. */

#define	DBC_CONTINUE	0x001		/* Continue dup search: next item. */
#define	DBC_KEYSET	0x002		/* Continue dup search: current item. */
#define	DBC_RECOVER	0x004		/* In recovery (do not log or lock). */
#define	DBC_RMW		0x008		/* Acquire write flag in read op. */
#define	DBC_WRITER	0x010		/* Cursor immediately writing (CDB). */
	u_int32_t flags;
};

/* Btree/recno statistics structure. */
struct __db_bt_stat {
	u_int32_t bt_flags;		/* Open flags. */
	u_int32_t bt_maxkey;		/* Maxkey value. */
	u_int32_t bt_minkey;		/* Minkey value. */
	u_int32_t bt_re_len;		/* Fixed-length record length. */
	u_int32_t bt_re_pad;		/* Fixed-length record pad. */
	u_int32_t bt_pagesize;		/* Page size. */
	u_int32_t bt_levels;		/* Tree levels. */
	u_int32_t bt_nrecs;		/* Number of records. */
	u_int32_t bt_int_pg;		/* Internal pages. */
	u_int32_t bt_leaf_pg;		/* Leaf pages. */
	u_int32_t bt_dup_pg;		/* Duplicate pages. */
	u_int32_t bt_over_pg;		/* Overflow pages. */
	u_int32_t bt_free;		/* Pages on the free list. */
	u_int32_t bt_int_pgfree;	/* Bytes free in internal pages. */
	u_int32_t bt_leaf_pgfree;	/* Bytes free in leaf pages. */
	u_int32_t bt_dup_pgfree;	/* Bytes free in duplicate pages. */
	u_int32_t bt_over_pgfree;	/* Bytes free in overflow pages. */
	u_int32_t bt_magic;		/* Magic number. */
	u_int32_t bt_version;		/* Version number. */
};

/* Hash statistics structure. */
struct __db_h_stat {
	u_int32_t hash_accesses;	/* Number of accesses to this table. */
	u_int32_t hash_collisions;	/* Number of collisions on search. */
	u_int32_t hash_expansions;	/* Number of times we added a bucket. */
	u_int32_t hash_overflows;	/* Number of overflow pages. */
	u_int32_t hash_bigpages;	/* Number of big key/data pages. */
	u_int32_t hash_dup;		/* Number of dup pages. */
	u_int32_t hash_free;		/* Pages on the free list. */
	u_int32_t hash_bfree;		/* Bytes free on bucket pages. */
	u_int32_t hash_dup_free;	/* Bytes free on duplicate pages. */
	u_int32_t hash_big_bfree;	/* Bytes free on big item pages. */
	u_int32_t hash_buckets;		/* Number of hash buckets. */
	u_int32_t hash_put;		/* Number of puts. */
	u_int32_t hash_deleted;		/* Number of deletes. */
	u_int32_t hash_get;		/* Number of gets. */
	u_int32_t hash_magic;		/* Magic number. */
	u_int32_t hash_version;		/* Version number. */
	u_int32_t hash_pagesize;	/* Page size. */
	u_int32_t hash_nrecs;		/* Number of records. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int   db_appinit (const char *, char * const *, DB_ENV *, u_int32_t);
int   db_appexit (DB_ENV *);
int   db_jump_set (void *, int);
int   db_open (const char *,
	       DBTYPE, u_int32_t, int, DB_ENV *, DB_INFO *, DB **);
int   db_value_set (int, int);
char *db_version (int *, int *, int *);
int   db_xa_open (const char *, DBTYPE, u_int32_t, int, DB_INFO *, DB **);
#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Locking
 *******************************************************/
#define	DB_LOCKVERSION	1
#define	DB_LOCKMAGIC	0x090193

/* Flag values for lock_vec(), lock_get(). */
#define	DB_LOCK_NOWAIT		0x01	/* Don't wait on unavailable lock. */
#define	DB_LOCK_UPGRADE		0x02	/* Upgrade an existing lock instead
					   of granting a new one (internal). */

/* Flag values for lock_detect(). */
#define	DB_LOCK_CONFLICT	0x01	/* Run on any conflict. */

/*
 * Request types.
 *
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_LOCK_DUMP=0,			/* Display held locks. */
	DB_LOCK_GET,			/* Get the lock. */
	DB_LOCK_INHERIT,		/* Pass locks to parent. */
	DB_LOCK_PUT,			/* Release the lock. */
	DB_LOCK_PUT_ALL,		/* Release locker's locks. */
	DB_LOCK_PUT_OBJ			/* Release locker's locks on obj. */
} db_lockop_t;

/*
 * Simple R/W lock modes and for multi-granularity intention locking.
 *
 * !!!
 * These values are NOT random, as they are used as an index into the lock
 * conflicts arrays, i.e., DB_LOCK_IWRITE must be == 3, and DB_LOCK_IREAD
 * must be == 4.
 *
 * !!!
 * Changes here must be reflected in java/src/com/sleepycat/db/Db.java.
 */
typedef enum {
	DB_LOCK_NG=0,			/* Not granted. */
	DB_LOCK_READ,			/* Shared/read. */
	DB_LOCK_WRITE,			/* Exclusive/write. */
	DB_LOCK_IWRITE,			/* Intent exclusive/write. */
	DB_LOCK_IREAD,			/* Intent to share/read. */
	DB_LOCK_IWR			/* Intent to read and write. */
} db_lockmode_t;

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

/* Lock request structure. */
struct __db_lockreq {
	db_lockop_t	 op;		/* Operation. */
	db_lockmode_t	 mode;		/* Requested mode. */
	u_int32_t	 locker;	/* Locker identity. */
	DBT		*obj;		/* Object being locked. */
	DB_LOCK		 lock;		/* Lock returned. */
};

/*
 * Commonly used conflict matrices.
 *
 * Standard Read/Write (or exclusive/shared) locks.
 */
#define	DB_LOCK_RW_N	3
extern const u_int8_t db_rw_conflicts[];

/* Multi-granularity locking. */
#define	DB_LOCK_RIW_N	6
extern const u_int8_t db_riw_conflicts[];

struct __db_lock_stat {
	u_int32_t st_magic;		/* Lock file magic number. */
	u_int32_t st_version;		/* Lock file version number. */
	u_int32_t st_maxlocks;		/* Maximum number of locks in table. */
	u_int32_t st_nmodes;		/* Number of lock modes. */
	u_int32_t st_numobjs;		/* Number of objects. */
	u_int32_t st_nlockers;		/* Number of lockers. */
	u_int32_t st_nconflicts;	/* Number of lock conflicts. */
	u_int32_t st_nrequests;		/* Number of lock gets. */
	u_int32_t st_nreleases;		/* Number of lock puts. */
	u_int32_t st_ndeadlocks;	/* Number of lock deadlocks. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_refcnt;		/* Region reference count. */
	u_int32_t st_regsize;		/* Region size. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	  lock_close (DB_LOCKTAB *);
int	  lock_detect (DB_LOCKTAB *, u_int32_t, u_int32_t);
int	  lock_get (DB_LOCKTAB *,
	    u_int32_t, u_int32_t, const DBT *, db_lockmode_t, DB_LOCK *);
int	  lock_id (DB_LOCKTAB *, u_int32_t *);
int	  lock_open (const char *, u_int32_t, int, DB_ENV *, DB_LOCKTAB **);
int	  lock_put (DB_LOCKTAB *, DB_LOCK);
int	  lock_tget (DB_LOCKTAB *,
	    DB_TXN *, u_int32_t, const DBT *, db_lockmode_t, DB_LOCK *);
int	  lock_stat (DB_LOCKTAB *, DB_LOCK_STAT **, void *(*)(size_t));
int	  lock_unlink (const char *, int, DB_ENV *);
int	  lock_vec (DB_LOCKTAB *,
	    u_int32_t, u_int32_t, DB_LOCKREQ *, int, DB_LOCKREQ **);
int	  lock_tvec (DB_LOCKTAB *,
	    DB_TXN *, u_int32_t, DB_LOCKREQ *, int, DB_LOCKREQ **);
#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Logging.
 *******************************************************/
/* Flag values for log_archive(). */
#define	DB_ARCH_ABS		0x001	/* Absolute pathnames. */
#define	DB_ARCH_DATA		0x002	/* Data files. */
#define	DB_ARCH_LOG		0x004	/* Log files. */

/*
 * A DB_LSN has two parts, a fileid which identifies a specific file, and an
 * offset within that file.  The fileid is an unsigned 4-byte quantity that
 * uniquely identifies a file within the log directory -- currently a simple
 * counter inside the log.  The offset is also an unsigned 4-byte value.  The
 * log manager guarantees the offset is never more than 4 bytes by switching
 * to a new log file before the maximum length imposed by an unsigned 4-byte
 * offset is reached.
 */
struct __db_lsn {
	u_int32_t	file;		/* File ID. */
	u_int32_t	offset;		/* File offset. */
};

/* Log statistics structure. */
struct __db_log_stat {
	u_int32_t st_magic;		/* Log file magic number. */
	u_int32_t st_version;		/* Log file version number. */
	int st_mode;			/* Log file mode. */
	u_int32_t st_lg_max;		/* Maximum log file size. */
	u_int32_t st_w_bytes;		/* Bytes to log. */
	u_int32_t st_w_mbytes;		/* Megabytes to log. */
	u_int32_t st_wc_bytes;		/* Bytes to log since checkpoint. */
	u_int32_t st_wc_mbytes;		/* Megabytes to log since checkpoint. */
	u_int32_t st_wcount;		/* Total syncs to the log. */
	u_int32_t st_scount;		/* Total writes to the log. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_cur_file;		/* Current log file number. */
	u_int32_t st_cur_offset;	/* Current log file offset. */
	u_int32_t st_refcnt;		/* Region reference count. */
	u_int32_t st_regsize;		/* Region size. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	 log_archive (DB_LOG *, char **[], u_int32_t, void *(*)(size_t));
int	 log_close (DB_LOG *);
int	 log_compare (const DB_LSN *, const DB_LSN *);
int	 log_file (DB_LOG *, const DB_LSN *, char *, size_t);
int	 log_flush (DB_LOG *, const DB_LSN *);
int	 log_get (DB_LOG *, DB_LSN *, DBT *, u_int32_t);
int	 log_open (const char *, u_int32_t, int, DB_ENV *, DB_LOG **);
int	 log_put (DB_LOG *, DB_LSN *, const DBT *, u_int32_t);
int	 log_register (DB_LOG *, DB *, const char *, DBTYPE, u_int32_t *);
int	 log_stat (DB_LOG *, DB_LOG_STAT **, void *(*)(size_t));
int	 log_unlink (const char *, int, DB_ENV *);
int	 log_unregister (DB_LOG *, u_int32_t);
#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Mpool
 *******************************************************/
/* Flag values for memp_fget(). */
#define	DB_MPOOL_CREATE		0x001	/* Create a page. */
#define	DB_MPOOL_LAST		0x002	/* Return the last page. */
#define	DB_MPOOL_NEW		0x004	/* Create a new page. */

/* Flag values for memp_fput(), memp_fset(). */
#define	DB_MPOOL_CLEAN		0x001	/* Clear modified bit. */
#define	DB_MPOOL_DIRTY		0x002	/* Page is modified. */
#define	DB_MPOOL_DISCARD	0x004	/* Don't cache the page. */

/* Mpool statistics structure. */
struct __db_mpool_stat {
	size_t st_cachesize;		/* Cache size. */
	u_int32_t st_cache_hit;		/* Pages found in the cache. */
	u_int32_t st_cache_miss;	/* Pages not found in the cache. */
	u_int32_t st_map;		/* Pages from mapped files. */
	u_int32_t st_page_create;	/* Pages created in the cache. */
	u_int32_t st_page_in;		/* Pages read in. */
	u_int32_t st_page_out;		/* Pages written out. */
	u_int32_t st_ro_evict;		/* Clean pages forced from the cache. */
	u_int32_t st_rw_evict;		/* Dirty pages forced from the cache. */
	u_int32_t st_hash_buckets;	/* Number of hash buckets. */
	u_int32_t st_hash_searches;	/* Total hash chain searches. */
	u_int32_t st_hash_longest;	/* Longest hash chain searched. */
	u_int32_t st_hash_examined;	/* Total hash entries searched. */
	u_int32_t st_page_clean;	/* Clean pages. */
	u_int32_t st_page_dirty;	/* Dirty pages. */
	u_int32_t st_page_trickle;	/* Pages written by memp_trickle. */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_refcnt;		/* Region reference count. */
	u_int32_t st_regsize;		/* Region size. */
};

/* Mpool file open information structure. */
struct __db_mpool_finfo {
	int	   ftype;		/* File type. */
	DBT	  *pgcookie;		/* Byte-string passed to pgin/pgout. */
	u_int8_t  *fileid;		/* Unique file ID. */
	int32_t	   lsn_offset;		/* LSN offset in page. */
	u_int32_t  clear_len;		/* Cleared length on created pages. */
};

/* Mpool file statistics structure. */
struct __db_mpool_fstat {
	char *file_name;		/* File name. */
	size_t st_pagesize;		/* Page size. */
	u_int32_t st_cache_hit;		/* Pages found in the cache. */
	u_int32_t st_cache_miss;	/* Pages not found in the cache. */
	u_int32_t st_map;		/* Pages from mapped files. */
	u_int32_t st_page_create;	/* Pages created in the cache. */
	u_int32_t st_page_in;		/* Pages read in. */
	u_int32_t st_page_out;		/* Pages written out. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	memp_close (DB_MPOOL *);
int	memp_fclose (DB_MPOOLFILE *);
int	memp_fget (DB_MPOOLFILE *, db_pgno_t *, u_int32_t, void *);
int	memp_fopen (DB_MPOOL *, const char *,
	    u_int32_t, int, size_t, DB_MPOOL_FINFO *, DB_MPOOLFILE **);
int	memp_fput (DB_MPOOLFILE *, void *, u_int32_t);
int	memp_fset (DB_MPOOLFILE *, void *, u_int32_t);
int	memp_fsync (DB_MPOOLFILE *);
int	memp_open (const char *, u_int32_t, int, DB_ENV *, DB_MPOOL **);
int	memp_register (DB_MPOOL *, int,
	    int (*)(db_pgno_t, void *, DBT *),
	    int (*)(db_pgno_t, void *, DBT *));
int	memp_stat (DB_MPOOL *,
	    DB_MPOOL_STAT **, DB_MPOOL_FSTAT ***, void *(*)(size_t));
int	memp_sync (DB_MPOOL *, DB_LSN *);
int	memp_trickle (DB_MPOOL *, int, int *);
int	memp_unlink (const char *, int, DB_ENV *);
#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Transactions.
 *******************************************************/
#define	DB_TXNVERSION	1
#define	DB_TXNMAGIC	0x041593

/* Operations values to the tx_recover() function. */
#define	DB_TXN_BACKWARD_ROLL	1	/* Read the log backwards. */
#define	DB_TXN_FORWARD_ROLL	2	/* Read the log forwards. */
#define	DB_TXN_OPENFILES	3	/* Read for open files. */
#define	DB_TXN_REDO		4	/* Redo the operation. */
#define	DB_TXN_UNDO		5	/* Undo the operation. */

/* Internal transaction status values. */

/* Transaction statistics structure. */
struct __db_txn_active {
	u_int32_t	txnid;		/* Transaction ID */
	DB_LSN		lsn;		/* Lsn of the begin record */
};

struct __db_txn_stat {
	DB_LSN	  st_last_ckp;		/* lsn of the last checkpoint */
	DB_LSN	  st_pending_ckp;	/* last checkpoint did not finish */
	time_t	  st_time_ckp;		/* time of last checkpoint */
	u_int32_t st_last_txnid;	/* last transaction id given out */
	u_int32_t st_maxtxns;	/* maximum number of active txns */
	u_int32_t st_naborts;	/* number of aborted transactions */
	u_int32_t st_nbegins;	/* number of begun transactions */
	u_int32_t st_ncommits;	/* number of committed transactions */
	u_int32_t st_nactive;	/* number of active transactions */
	DB_TXN_ACTIVE
		 *st_txnarray;	/* array of active transactions */
	u_int32_t st_region_wait;	/* Region lock granted after wait. */
	u_int32_t st_region_nowait;	/* Region lock granted without wait. */
	u_int32_t st_refcnt;		/* Region reference count. */
	u_int32_t st_regsize;		/* Region size. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	  txn_abort (DB_TXN *);
int	  txn_begin (DB_TXNMGR *, DB_TXN *, DB_TXN **);
int	  txn_checkpoint (const DB_TXNMGR *, u_int32_t, u_int32_t);
int	  txn_commit (DB_TXN *);
int	  txn_close (DB_TXNMGR *);
u_int32_t txn_id (DB_TXN *);
int	  txn_open (const char *, u_int32_t, int, DB_ENV *, DB_TXNMGR **);
int	  txn_prepare (DB_TXN *);
int	  txn_stat (DB_TXNMGR *, DB_TXN_STAT **, void *(*)(size_t));
int	  txn_unlink (const char *, int, DB_ENV *);
#if defined(__cplusplus)
}
#endif

#ifndef DB_DBM_HSEARCH
#define	DB_DBM_HSEARCH	0		/* No historic interfaces by default. */
#endif
#if DB_DBM_HSEARCH != 0
/*******************************************************
 * Dbm/Ndbm historic interfaces.
 *******************************************************/
#define	DBM_INSERT	0		/* Flags to dbm_store(). */
#define	DBM_REPLACE	1

/*
 * The db(3) support for ndbm(3) always appends this suffix to the
 * file name to avoid overwriting the user's original database.
 */
#define	DBM_SUFFIX	".db"

#if defined(_XPG4_2)
typedef struct {
	char *dptr;
	size_t dsize;
} datum;
#else
typedef struct {
	char *dptr;
	int dsize;
} datum;
#endif

/*
 * Translate DBM calls into DB calls so that DB doesn't step on the
 * application's name space.
 *
 * The global variables dbrdonly, dirf and pagf were not retained when
 * 4BSD replaced the dbm interface with ndbm, and are not support here.
 */
#define	dbminit(a)	__db_dbm_init(a)
#define	dbmclose	__db_dbm_close
#if !defined(__cplusplus)
#define	delete(a)	__db_dbm_delete(a)
#endif
#define	fetch(a)	__db_dbm_fetch(a)
#define	firstkey	__db_dbm_firstkey
#define	nextkey(a)	__db_dbm_nextkey(a)
#define	store(a, b)	__db_dbm_store(a, b)

/* Prototype the DB calls. */
#if defined(__cplusplus)
extern "C" {
#endif
int	 __db_dbm_close (void);
int	 __db_dbm_dbrdonly (void);
int	 __db_dbm_delete (datum);
int	 __db_dbm_dirf (void);
datum	 __db_dbm_fetch (datum);
datum	 __db_dbm_firstkey (void);
int	 __db_dbm_init (char *);
datum	 __db_dbm_nextkey (datum);
int	 __db_dbm_pagf (void);
int	 __db_dbm_store (datum, datum);
#if defined(__cplusplus)
}
#endif

/*
 * Translate NDBM calls into DB calls so that DB doesn't step on the
 * application's name space.
 */
#define	dbm_clearerr(a)		__db_ndbm_clearerr(a)
#define	dbm_close(a)		__db_ndbm_close(a)
#define	dbm_delete(a, b)	__db_ndbm_delete(a, b)
#define	dbm_dirfno(a)		__db_ndbm_dirfno(a)
#define	dbm_error(a)		__db_ndbm_error(a)
#define	dbm_fetch(a, b)		__db_ndbm_fetch(a, b)
#define	dbm_firstkey(a)		__db_ndbm_firstkey(a)
#define	dbm_nextkey(a)		__db_ndbm_nextkey(a)
#define	dbm_open(a, b, c)	__db_ndbm_open(a, b, c)
#define	dbm_pagfno(a)		__db_ndbm_pagfno(a)
#define	dbm_rdonly(a)		__db_ndbm_rdonly(a)
#define	dbm_store(a, b, c, d)	__db_ndbm_store(a, b, c, d)

/* Prototype the DB calls. */
#if defined(__cplusplus)
extern "C" {
#endif
int	 __db_ndbm_clearerr (DBM *);
void	 __db_ndbm_close (DBM *);
int	 __db_ndbm_delete (DBM *, datum);
int	 __db_ndbm_dirfno (DBM *);
int	 __db_ndbm_error (DBM *);
datum	 __db_ndbm_fetch (DBM *, datum);
datum	 __db_ndbm_firstkey (DBM *);
datum	 __db_ndbm_nextkey (DBM *);
DBM	*__db_ndbm_open (const char *, int, int);
int	 __db_ndbm_pagfno (DBM *);
int	 __db_ndbm_rdonly (DBM *);
int	 __db_ndbm_store (DBM *, datum, datum, int);
#if defined(__cplusplus)
}
#endif

/*******************************************************
 * Hsearch historic interface.
 *******************************************************/
typedef enum {
	FIND, ENTER
} ACTION;

typedef struct entry {
	char *key;
	char *data;
} ENTRY;

/*
 * Translate HSEARCH calls into DB calls so that DB doesn't step on the
 * application's name space.
 */
#define	hcreate(a)	__db_hcreate(a)
#define	hdestroy	__db_hdestroy
#define	hsearch(a, b)	__db_hsearch(a, b)

/* Prototype the DB calls. */
#if defined(__cplusplus)
extern "C" {
#endif
int	 __db_hcreate (size_t);
void	 __db_hdestroy (void);
ENTRY	*__db_hsearch (ENTRY, ACTION);
#if defined(__cplusplus)
}
#endif
#endif /* DB_DBM_HSEARCH */

/*
 * XXX
 * MacOS: Reset Metrowerks C enum sizes.
 */
#ifdef __MWERKS__
#pragma enumsalwaysint reset
#endif
#endif /* !_DB_H_ */
