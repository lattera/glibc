/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)db.h.src	10.77 (Sleepycat) 9/24/97
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
 */
#undef	__P
#if defined(__STDC__) || defined(__cplusplus)
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
 * by the system.  This isn't completely necessary, but the example programs
 * need them.
 */
#ifndef	__BIT_TYPES_DEFINED__
#define	__BIT_TYPES_DEFINED__





#endif






#define	DB_VERSION_MAJOR	2
#define	DB_VERSION_MINOR	3
#define	DB_VERSION_PATCH	10
#define	DB_VERSION_STRING	"Sleepycat Software: DB 2.3.10: (9/24/97)"

typedef	u_int32_t	db_pgno_t;	/* Page number type. */
typedef	u_int16_t	db_indx_t;	/* Page offset type. */
#define	DB_MAX_PAGES	0xffffffff	/* >= # of pages in a file */

typedef	u_int32_t	db_recno_t;	/* Record number type. */
typedef size_t		DB_LOCK;	/* Object returned by lock manager. */
#define	DB_MAX_RECORDS	0xffffffff	/* >= # of records in a tree */

#define	DB_FILE_ID_LEN		20	/* DB file ID length. */

/* Forward structure declarations, so applications get type checking. */
struct __db;		typedef struct __db DB;
#ifdef DB_DBM_HSEARCH
			typedef struct __db DBM;
#endif
struct __db_bt_stat;	typedef struct __db_bt_stat DB_BTREE_STAT;
struct __db_dbt;	typedef struct __db_dbt DBT;
struct __db_env;	typedef struct __db_env DB_ENV;
struct __db_info;	typedef struct __db_info DB_INFO;
struct __db_lockregion;	typedef struct __db_lockregion DB_LOCKREGION;
struct __db_lockreq;	typedef struct __db_lockreq DB_LOCKREQ;
struct __db_locktab;	typedef struct __db_locktab DB_LOCKTAB;
struct __db_log;	typedef struct __db_log DB_LOG;
struct __db_lsn;	typedef struct __db_lsn DB_LSN;
struct __db_mpool;	typedef struct __db_mpool DB_MPOOL;
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

#define	DB_DBT_INTERNAL	0x01		/* Perform any mallocs using regular
					   malloc, not the user's malloc. */
#define	DB_DBT_MALLOC	0x02		/* Return in allocated memory. */
#define	DB_DBT_PARTIAL	0x04		/* Partial put/get. */
#define	DB_DBT_USERMEM	0x08		/* Return in user's memory. */
	u_int32_t flags;
};

/*
 * Database configuration and initialization.
 */
 /*
  * Flags understood by both db_open(3) and db_appinit(3).
  */
#define	DB_CREATE		0x00001	/* O_CREAT: create file as necessary. */
#define	DB_NOMMAP		0x00002	/* Don't mmap underlying file. */
#define	DB_THREAD		0x00004	/* Free-thread DB package handles. */

/*
 * Flags understood by db_appinit(3).
 *
 * DB_APP_INIT and DB_MUTEXDEBUG are internal only, and not documented.
 */
/*				0x00007	   COMMON MASK. */
#define	DB_APP_INIT		0x00008	/* Appinit called, paths initialized. */
#define	DB_INIT_LOCK		0x00010	/* Initialize locking. */
#define	DB_INIT_LOG		0x00020	/* Initialize logging. */
#define	DB_INIT_MPOOL		0x00040	/* Initialize mpool. */
#define	DB_INIT_TXN		0x00080	/* Initialize transactions. */
#define	DB_MPOOL_PRIVATE	0x00100	/* Mpool: private memory pool. */
#define	DB_MUTEXDEBUG		0x00200	/* Do not get/set mutexes in regions. */
#define	DB_RECOVER		0x00400	/* Run normal recovery. */
#define	DB_RECOVER_FATAL	0x00800 /* Run catastrophic recovery. */
#define	DB_TXN_NOSYNC		0x01000	/* Do not sync log on commit. */
#define	DB_USE_ENVIRON		0x02000	/* Use the environment. */
#define	DB_USE_ENVIRON_ROOT	0x04000	/* Use the environment if root. */

/* CURRENTLY UNUSED LOCK FLAGS. */
#define	DB_TXN_LOCK_2PL		0x00000	/* Two-phase locking. */
#define	DB_TXN_LOCK_OPTIMISTIC	0x00000	/* Optimistic locking. */
#define	DB_TXN_LOCK_MASK	0x00000	/* Lock flags mask. */

/* CURRENTLY UNUSED LOG FLAGS. */
#define	DB_TXN_LOG_REDO		0x00000	/* Redo-only logging. */
#define	DB_TXN_LOG_UNDO		0x00000	/* Undo-only logging. */
#define	DB_TXN_LOG_UNDOREDO	0x00000	/* Undo/redo write-ahead logging. */
#define	DB_TXN_LOG_MASK		0x00000	/* Log flags mask. */

/*
 * Flags understood by db_open(3).
 *
 * DB_EXCL and DB_TEMPORARY are internal only, and not documented.
 * DB_SEQUENTIAL is currently internal, but likely to be exported some day.
 */
/*				0x00007	   COMMON MASK. */
/*				0x07fff	   ALREADY USED. */
#define	DB_EXCL			0x08000	/* O_EXCL: exclusive open. */
#define	DB_RDONLY		0x10000	/* O_RDONLY: read-only. */
#define	DB_SEQUENTIAL		0x20000	/* Indicate sequential access. */
#define	DB_TEMPORARY		0x40000	/* Remove on last close. */
#define	DB_TRUNCATE		0x80000	/* O_TRUNCATE: replace existing DB. */

/*
 * Deadlock detector modes; used in the DBENV structure to configure the
 * locking subsystem.
 */
#define	DB_LOCK_NORUN		0x0
#define	DB_LOCK_DEFAULT		0x1
#define	DB_LOCK_OLDEST		0x2
#define	DB_LOCK_RANDOM		0x3
#define	DB_LOCK_YOUNGEST	0x4

struct __db_env {
	int		 db_lorder;	/* Byte order. */

					/* Error message callback. */
	void (*db_errcall) __P((const char *, char *));
	FILE		*db_errfile;	/* Error message file stream. */
	const char	*db_errpfx;	/* Error message prefix. */
	int		 db_verbose;	/* Generate debugging messages. */

	/* User paths. */
	char		*db_home;	/* Database home. */
	char		*db_log_dir;	/* Database log file directory. */
	char		*db_tmp_dir;	/* Database tmp file directory. */

	char	       **db_data_dir;	/* Database data file directories. */
	int		 data_cnt;	/* Database data file slots. */
	int		 data_next;	/* Next Database data file slot. */

	/* Locking. */
	DB_LOCKTAB	*lk_info;	/* Return from lock_open(). */
	u_int8_t	*lk_conflicts;	/* Two dimensional conflict matrix. */
	int		 lk_modes;	/* Number of lock modes in table. */
	unsigned int	 lk_max;	/* Maximum number of locks. */
	u_int32_t	 lk_detect;	/* Deadlock detect on every conflict. */
	int (*db_yield) __P((void));	/* Yield function for threads. */

	/* Logging. */
	DB_LOG		*lg_info;	/* Return from log_open(). */
	u_int32_t	 lg_max;	/* Maximum file size. */

	/* Memory pool. */
	DB_MPOOL	*mp_info;	/* Return from memp_open(). */
	size_t		 mp_mmapsize;	/* Maximum file size for mmap. */
	size_t		 mp_size;	/* Bytes in the mpool cache. */

	/* Transactions. */
	DB_TXNMGR	*tx_info;	/* Return from txn_open(). */
	unsigned int	 tx_max;	/* Maximum number of transactions. */
	int (*tx_recover)		/* Dispatch function for recovery. */
	    __P((DB_LOG *, DBT *, DB_LSN *, int, void *));

	u_int32_t	 flags;		/* Flags. */
};

/*******************************************************
 * Access methods.
 *******************************************************/
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
	void *(*db_malloc) __P((size_t));

	/* Btree access method. */
	int		 bt_maxkey;	/* Maximum keys per page. */
	int		 bt_minkey;	/* Minimum keys per page. */
	int (*bt_compare)		/* Comparison function. */
	    __P((const DBT *, const DBT *));
	size_t (*bt_prefix)		/* Prefix function. */
	    __P((const DBT *, const DBT *));

	/* Hash access method. */
	unsigned int	 h_ffactor;	/* Fill factor. */
	unsigned int	 h_nelem;	/* Number of elements. */
	u_int32_t	(*h_hash)	/* Hash function. */
	    __P((const void *, u_int32_t));

	/* Recno access method. */
	int		 re_pad;	/* Fixed-length padding byte. */
	int		 re_delim;	/* Variable-length delimiting byte. */
	u_int32_t	 re_len;	/* Length for fixed-length records. */
	char		*re_source;	/* Source file name. */

#define	DB_DELIMITER		0x0001	/* Recno: re_delim set. */
#define	DB_DUP			0x0002	/* Btree, Hash: duplicate keys. */
#define	DB_FIXEDLEN		0x0004	/* Recno: fixed-length records. */
#define	DB_PAD			0x0008	/* Recno: re_pad set. */
#define	DB_RECNUM		0x0010	/* Btree: record numbers. */
#define	DB_RENUMBER		0x0020	/* Recno: renumber on insert/delete. */
#define	DB_SNAPSHOT		0x0040	/* Recno: snapshot the input. */
	u_int32_t	 flags;
};

/*
 * DB access method and cursor operation codes.  These are implemented as
 * bit fields for future flexibility, but currently only a single one may
 * be specified to any function.
 */
#define	DB_AFTER	0x000001	/* c_put() */
#define	DB_APPEND	0x000002	/* put() */
#define	DB_BEFORE	0x000004	/* c_put() */
#define	DB_CHECKPOINT	0x000008	/* log_put(), log_get() */
#define	DB_CURRENT	0x000010	/* c_get(), c_put(), log_get() */
#define	DB_FIRST	0x000020	/* c_get(), log_get() */
#define	DB_FLUSH	0x000040	/* log_put() */
#define	DB_GET_RECNO	0x000080	/* c_get() */
#define	DB_KEYFIRST	0x000100	/* c_put() */
#define	DB_KEYLAST	0x000200	/* c_put() */
#define	DB_LAST		0x000400	/* c_get(), log_get() */
#define	DB_NEXT		0x000800	/* c_get(), log_get() */
#define	DB_NOOVERWRITE	0x001000	/* put() */
#define	DB_NOSYNC	0x002000	/* close() */
#define	DB_PREV		0x004000	/* c_get(), log_get() */
#define	DB_RECORDCOUNT	0x008000	/* stat() */
#define	DB_SET		0x010000	/* c_get(), log_get() */
#define	DB_SET_RANGE	0x020000	/* c_get() */
#define	DB_SET_RECNO	0x040000	/* get(), c_get() */

/* DB (user visible) error return codes. */
#define	DB_INCOMPLETE		( -1)	/* Sync didn't finish. */
#define	DB_KEYEMPTY		( -2)	/* The key/data pair was deleted or
					   was never created by the user. */
#define	DB_KEYEXIST		( -3)	/* The key/data pair already exists. */
#define	DB_LOCK_DEADLOCK	( -4)	/* Locker killed to resolve deadlock. */
#define	DB_LOCK_NOTGRANTED	( -5)	/* Lock unavailable, no-wait set. */
#define	DB_LOCK_NOTHELD		( -6)	/* Lock not held by locker. */
#define	DB_NOTFOUND		( -7)	/* Key/data pair not found (EOF). */

/* DB (private) error return codes. */
#define	DB_DELETED		( -8)	/* Recovery file marked deleted. */
#define	DB_NEEDSPLIT		( -9)	/* Page needs to be split. */
#define	DB_REGISTERED		(-10)	/* Entry was previously registered. */
#define	DB_SWAPBYTES		(-11)	/* Database needs byte swapping. */
#define DB_TXN_CKP		(-12)	/* Encountered ckp record in log. */

struct __db_ilock {			/* Internal DB access method lock. */
	db_pgno_t	pgno;		/* Page being locked. */
					/* File id. */
	u_int8_t	fileid[DB_FILE_ID_LEN];
};

/* DB access method description structure. */
struct __db {
	void	*mutexp;		/* Synchronization for free threading */
	DBTYPE	 type;			/* DB access method. */
	DB_ENV	*dbenv;			/* DB_ENV structure. */
	DB_ENV	*mp_dbenv;		/* DB_ENV for local mpool creation. */

	DB	*master;		/* Original DB created by db_open. */
	void	*internal;		/* Access method private. */

	DB_MPOOL	*mp;		/* The access method's mpool. */
	DB_MPOOLFILE	*mpf;		/* The access method's mpool file. */

	/*
	 * XXX
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_HEAD(curs_queue, __dbc);
	 */
	struct {
		struct __dbc *tqh_first;
		struct __dbc **tqh_last;
	} curs_queue;

	/*
	 * XXX
	 * Explicit representations of structures in queue.h.
	 *
	 * LIST_HEAD(handleq, __db);
	 * LIST_ENTRY(__db);
	 */
	struct {
		struct __db *lh_first;
	} handleq;			/* List of handles for this DB. */
	struct {
		struct __db *le_next;
		struct __db **le_prev;
	} links;			/* Links for the handle list. */

	u_int32_t log_fileid;		/* Logging file id. */

	DB_TXN	 *txn;			/* Current transaction. */
	u_int32_t locker;		/* Default process' locker id. */
	DBT	  lock_dbt;		/* DBT referencing lock. */
	struct __db_ilock lock;		/* Lock. */

	size_t	  pgsize;		/* Logical page size of file. */

					/* Local heap allocation. */
	void *(*db_malloc) __P((size_t));

					/* Functions. */
	int (*close)	__P((DB *, int));
	int (*cursor)	__P((DB *, DB_TXN *, DBC **));
	int (*del)	__P((DB *, DB_TXN *, DBT *, int));
	int (*fd)	__P((DB *, int *));
	int (*get)	__P((DB *, DB_TXN *, DBT *, DBT *, int));
	int (*put)	__P((DB *, DB_TXN *, DBT *, DBT *, int));
	int (*stat)	__P((DB *, void *, void *(*)(size_t), int));
	int (*sync)	__P((DB *, int));

#define	DB_AM_DUP	0x000001	/* DB_DUP (internal). */
#define	DB_AM_INMEM	0x000002	/* In-memory; no sync on close. */
#define	DB_AM_LOCKING	0x000004	/* Perform locking. */
#define	DB_AM_LOGGING	0x000008	/* Perform logging. */
#define	DB_AM_MLOCAL	0x000010	/* Database memory pool is local. */
#define	DB_AM_PGDEF	0x000020	/* Page size was defaulted. */
#define	DB_AM_RDONLY	0x000040	/* Database is readonly. */
#define	DB_AM_RECOVER	0x000080	/* In recovery (do not log or lock). */
#define	DB_AM_SWAP	0x000100	/* Pages need to be byte-swapped. */
#define	DB_AM_THREAD	0x000200	/* DB is multi-threaded. */
#define	DB_BT_RECNUM	0x000400	/* DB_RECNUM (internal) */
#define	DB_HS_DIRTYMETA 0x000800	/* Hash: Metadata page modified. */
#define	DB_RE_DELIMITER	0x001000	/* DB_DELIMITER (internal). */
#define	DB_RE_FIXEDLEN	0x002000	/* DB_FIXEDLEN (internal). */
#define	DB_RE_PAD	0x004000	/* DB_PAD (internal). */
#define	DB_RE_RENUMBER	0x008000	/* DB_RENUMBER (internal). */
#define	DB_RE_SNAPSHOT	0x010000	/* DB_SNAPSHOT (internal). */

	u_int32_t flags;
};

/* Cursor description structure. */
struct __dbc {
	DB *dbp;			/* Related DB access method. */
	DB_TXN	 *txn;			/* Associated transaction. */

	/*
	 * XXX
	 * Explicit representations of structures in queue.h.
	 *
	 * TAILQ_ENTRY(__dbc);
	 */
	struct {
		struct __dbc *tqe_next;
		struct __dbc **tqe_prev;
	} links;

	void	 *internal;		/* Access method private. */

	int (*c_close)	__P((DBC *));
	int (*c_del)	__P((DBC *, int));
	int (*c_get)	__P((DBC *, DBT *, DBT *, int));
	int (*c_put)	__P((DBC *, DBT *, DBT *, int));
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
	u_int32_t bt_freed;		/* Pages freed for reuse. */
	u_int32_t bt_int_pgfree;	/* Bytes free in internal pages. */
	u_int32_t bt_leaf_pgfree;	/* Bytes free in leaf pages. */
	u_int32_t bt_dup_pgfree;	/* Bytes free in duplicate pages. */
	u_int32_t bt_over_pgfree;	/* Bytes free in overflow pages. */
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

#if defined(__cplusplus)
extern "C" {
#endif
int   db_appinit __P((const char *, char * const *, DB_ENV *, int));
int   db_appexit __P((DB_ENV *));
int   db_open __P((const char *, DBTYPE, int, int, DB_ENV *, DB_INFO *, DB **));
char *db_version __P((int *, int *, int *));
#if defined(__cplusplus)
};
#endif

/*******************************************************
 * Locking
 *******************************************************/
#define	DB_LOCKVERSION	1
#define	DB_LOCKMAGIC	0x090193

/* Flag values for lock_vec(). */
#define	DB_LOCK_NOWAIT		0x01	/* Don't wait on unavailable lock. */

/* Flag values for lock_detect(). */
#define	DB_LOCK_CONFLICT	0x01	/* Run on any conflict. */

/* Request types. */
typedef enum {
	DB_LOCK_DUMP,			/* Display held locks. */
	DB_LOCK_GET,			/* Get the lock. */
	DB_LOCK_PUT,			/* Release the lock. */
	DB_LOCK_PUT_ALL,		/* Release locker's locks. */
	DB_LOCK_PUT_OBJ			/* Release locker's locks on obj. */
} db_lockop_t;

/* Simple R/W lock modes and for multi-granularity intention locking. */
typedef enum {
	DB_LOCK_NG=0,			/* Not granted. */
	DB_LOCK_READ,			/* Shared/read. */
	DB_LOCK_WRITE,			/* Exclusive/write. */
	DB_LOCK_IREAD,			/* Intent to share/read. */
	DB_LOCK_IWRITE,			/* Intent exclusive/write. */
	DB_LOCK_IWR			/* Intent to read and write. */
} db_lockmode_t;

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

#if defined(__cplusplus)
extern "C" {
#endif
int	  lock_close __P((DB_LOCKTAB *));
int	  lock_detect __P((DB_LOCKTAB *, int, u_int32_t));
int	  lock_get __P((DB_LOCKTAB *,
	    u_int32_t, int, const DBT *, db_lockmode_t, DB_LOCK *));
int	  lock_id __P((DB_LOCKTAB *, u_int32_t *));
int	  lock_open __P((const char *, int, int, DB_ENV *, DB_LOCKTAB **));
int	  lock_put __P((DB_LOCKTAB *, DB_LOCK));
int	  lock_unlink __P((const char *, int, DB_ENV *));
int	  lock_vec __P((DB_LOCKTAB *,
	    u_int32_t, int, DB_LOCKREQ *, int, DB_LOCKREQ **));
#if defined(__cplusplus)
};
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

#if defined(__cplusplus)
extern "C" {
#endif
int	 log_archive __P((DB_LOG *, char **[], int, void *(*)(size_t)));
int	 log_close __P((DB_LOG *));
int	 log_compare __P((const DB_LSN *, const DB_LSN *));
int	 log_file __P((DB_LOG *, const DB_LSN *, char *, size_t));
int	 log_flush __P((DB_LOG *, const DB_LSN *));
int	 log_get __P((DB_LOG *, DB_LSN *, DBT *, int));
int	 log_open __P((const char *, int, int, DB_ENV *, DB_LOG **));
int	 log_put __P((DB_LOG *, DB_LSN *, const DBT *, int));
int	 log_register __P((DB_LOG *, DB *, const char *, DBTYPE, u_int32_t *));
int	 log_unlink __P((const char *, int, DB_ENV *));
int	 log_unregister __P((DB_LOG *, u_int32_t));
#if defined(__cplusplus)
};
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
	unsigned long st_cache_hit;	/* Pages found in the cache. */
	unsigned long st_cache_miss;	/* Pages not found in the cache. */
	unsigned long st_map;		/* Pages from mapped files. */
	unsigned long st_page_create;	/* Pages created in the cache. */
	unsigned long st_page_in;	/* Pages read in. */
	unsigned long st_page_out;	/* Pages written out. */
	unsigned long st_ro_evict;	/* Read-only pages evicted. */
	unsigned long st_rw_evict;	/* Read-write pages evicted. */
	unsigned long st_hash_buckets;	/* Number of hash buckets. */
	unsigned long st_hash_searches;	/* Total hash chain searches. */
	unsigned long st_hash_longest;	/* Longest hash chain searched. */
	unsigned long st_hash_examined;	/* Total hash entries searched. */
};

/* Mpool file statistics structure. */
struct __db_mpool_fstat {
	char *file_name;		/* File name. */
	size_t st_pagesize;		/* Page size. */
	unsigned long st_cache_hit;	/* Pages found in the cache. */
	unsigned long st_cache_miss;	/* Pages not found in the cache. */
	unsigned long st_map;		/* Pages from mapped files. */
	unsigned long st_page_create;	/* Pages created in the cache. */
	unsigned long st_page_in;	/* Pages read in. */
	unsigned long st_page_out;	/* Pages written out. */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	memp_close __P((DB_MPOOL *));
int	memp_fclose __P((DB_MPOOLFILE *));
int	memp_fget __P((DB_MPOOLFILE *, db_pgno_t *, int, void *));
int	memp_fopen __P((DB_MPOOL *, const char *,
	    int, int, int, size_t, int, DBT *, u_int8_t *, DB_MPOOLFILE **));
int	memp_fput __P((DB_MPOOLFILE *, void *, int));
int	memp_fset __P((DB_MPOOLFILE *, void *, int));
int	memp_fsync __P((DB_MPOOLFILE *));
int	memp_open __P((const char *, int, int, DB_ENV *, DB_MPOOL **));
int	memp_register __P((DB_MPOOL *, int,
	    int (*)(db_pgno_t, void *, DBT *),
	    int (*)(db_pgno_t, void *, DBT *)));
int	memp_stat __P((DB_MPOOL *,
	    DB_MPOOL_STAT **, DB_MPOOL_FSTAT ***, void *(*)(size_t)));
int	memp_sync __P((DB_MPOOL *, DB_LSN *));
int	memp_unlink __P((const char *, int, DB_ENV *));
#if defined(__cplusplus)
};
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
	DB_LSN		st_last_ckp;	/* lsn of the last checkpoint */
	DB_LSN		st_pending_ckp;	/* last checkpoint did not finish */
	time_t		st_time_ckp;	/* time of last checkpoint */
	u_int32_t	st_last_txnid;	/* last transaction id given out */
	u_int32_t	st_maxtxns;	/* maximum number of active txns */
	u_int32_t	st_naborts;	/* number of aborted transactions */
	u_int32_t	st_nbegins;	/* number of begun transactions */
	u_int32_t	st_ncommits;	/* number of committed transactions */
	u_int32_t	st_nactive;	/* number of active transactions */
	DB_TXN_ACTIVE	*st_txnarray;	/* array of active transactions */
};

#if defined(__cplusplus)
extern "C" {
#endif
int	  txn_abort __P((DB_TXN *));
int	  txn_begin __P((DB_TXNMGR *, DB_TXN *, DB_TXN **));
int	  txn_checkpoint __P((const DB_TXNMGR *, int, int));
int	  txn_commit __P((DB_TXN *));
int	  txn_close __P((DB_TXNMGR *));
u_int32_t txn_id __P((DB_TXN *));
int	  txn_open __P((const char *, int, int, DB_ENV *, DB_TXNMGR **));
int	  txn_prepare __P((DB_TXN *));
int	  txn_stat __P((DB_TXNMGR *, DB_TXN_STAT **, void *(*)(size_t)));
int	  txn_unlink __P((const char *, int, DB_ENV *));
#if defined(__cplusplus)
};
#endif

#ifdef DB_DBM_HSEARCH
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

typedef struct {
	char *dptr;
	int dsize;
} datum;

#if defined(__cplusplus)
extern "C" {
#endif
int	 dbminit __P((char *));
#if !defined(__cplusplus)
int	 delete __P((datum));
#endif
datum	 fetch __P((datum));
datum	 firstkey __P((void));
datum	 nextkey __P((datum));
int	 store __P((datum, datum));

/*
 * !!!
 * Don't prototype:
 *
 *	 dbm_clearerr(DBM *db);
 *	 dbm_dirfno(DBM *db);
 *	 dbm_error(DBM *db);
 *	 dbm_pagfno(DBM *db);
 *	 dbm_rdonly(DBM *db);
 *
 * they weren't documented and were historically implemented as #define's.
 */
void	 dbm_close __P((DBM *));
int	 dbm_delete __P((DBM *, datum));
datum	 dbm_fetch __P((DBM *, datum));
datum	 dbm_firstkey __P((DBM *));
long	 dbm_forder __P((DBM *, datum));
datum	 dbm_nextkey __P((DBM *));
DBM	*dbm_open __P((const char *, int, int));
int	 dbm_store __P((DBM *, datum, datum, int));
#if defined(__cplusplus)
};
#endif

/*******************************************************
 * Hsearch historic interface.
 *******************************************************/
typedef enum {
	FIND, ENTER
} ACTION;

typedef struct entry {
	char *key;
	void *data;
} ENTRY;

#if defined(__cplusplus)
extern "C" {
#endif
int	 hcreate __P((unsigned int));
void	 hdestroy __P((void));
ENTRY	*hsearch __P((ENTRY, ACTION));
#if defined(__cplusplus)
};
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
