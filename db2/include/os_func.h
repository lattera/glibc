/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)os_func.h	10.4 (Sleepycat) 11/28/97
 */

/* Calls which can be replaced by the application. */
struct __db_jumptab {
	int	(*db_close) __P((int));			/* DB_FUNC_CLOSE */
	void	(*db_dirfree) __P((char **, int));	/* DB_FUNC_DIRFREE */
	int	(*db_dirlist)				/* DB_FUNC_DIRLIST */
		    __P((const char *, char ***, int *));
	int	(*db_exists)				/* DB_FUNC_EXISTS */
		    __P((const char *, int *));
	void	(*db_free) __P((void *));		/* DB_FUNC_FREE */
	int	(*db_fsync) __P((int));			/* DB_FUNC_FSYNC */
	int	(*db_ioinfo)				/* DB_FUNC_IOINFO */
		    __P((const char *, int, off_t *, off_t *));
	void   *(*db_malloc) __P((size_t));		/* DB_FUNC_MALLOC */
	int	(*db_map)				/* DB_FUNC_MAP */
		    __P((int, size_t, int, int, void **));
	int	(*db_open)				/* DB_FUNC_OPEN */
		    __P((const char *, int, ...));
	ssize_t	(*db_read) __P((int, void *, size_t));	/* DB_FUNC_READ */
	void   *(*db_realloc) __P((void *, size_t));	/* DB_FUNC_REALLOC */
	int	(*db_seek)				/* DB_FUNC_SEEK */
		    __P((int, size_t, db_pgno_t, u_long, int));
	int	(*db_sleep) __P((u_long, u_long));	/* DB_FUNC_SLEEP */
	char   *(*db_strdup) __P((const char *));	/* DB_FUNC_STRDUP */
	int	(*db_unlink) __P((const char *));	/* DB_FUNC_UNLINK */
	int	(*db_unmap) __P((void *, size_t));	/* DB_FUNC_UNMAP */
	ssize_t	(*db_write)				/* DB_FUNC_WRITE */
		    __P((int, const void *, size_t));
	int	(*db_yield) __P((void));		/* DB_FUNC_YIELD */
};

extern struct __db_jumptab __db_jump;

/*
 * Names used by DB to call through the jump table.
 *
 * The naming scheme goes like this: if the functionality the application can
 * replace is the same as the DB functionality, e.g., calloc, or dirlist, then
 * we use the name __db_XXX, and the application is expected to replace the
 * complete functionality, which may or may not map directly to an ANSI C or
 * POSIX 1003.1 interface.  If the functionality that the aplication replaces
 * only underlies what the DB os directory exports to other parts of DB, e.g.,
 * read, then the name __os_XXX is used, and the application can only replace
 * the underlying functionality.  Under most circumstances, the os directory
 * part of DB is the only code that should use the __os_XXX names, all other
 * parts of DB should be calling __db_XXX functions.
 */
#define	__os_close	__db_jump.db_close	/* __db_close is a wrapper. */
#define	__db_dirfree	__db_jump.db_dirfree
#define	__db_dirlist	__db_jump.db_dirlist
#define	__db_exists	__db_jump.db_exists
#define	__db_free	__db_jump.db_free
#define	__os_fsync	__db_jump.db_fsync	/* __db_fsync is a wrapper. */
#define	__db_ioinfo	__db_jump.db_ioinfo
#define	__db_map	__db_jump.db_map
#define	__os_open	__db_jump.db_open	/* __db_open is a wrapper. */
#define	__os_read	__db_jump.db_read	/* __db_read is a wrapper. */
#define	__db_seek	__db_jump.db_seek
#define	__db_sleep	__db_jump.db_sleep
#define	__db_strdup	__db_jump.db_strdup
#define	__os_unlink	__db_jump.db_unlink	/* __db_unlink is a wrapper. */
#define	__db_unmap	__db_jump.db_unmap
#define	__os_write	__db_jump.db_write	/* __db_write is a wrapper. */
#define	__db_yield	__db_jump.db_yield
