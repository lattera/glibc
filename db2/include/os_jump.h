/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 *
 *	@(#)os_jump.h	10.1 (Sleepycat) 10/17/98
 */

/* Calls which can be replaced by the application. */
struct __db_jumptab {
	int	(*j_close) __P((int));			/* DB_FUNC_CLOSE */
	void	(*j_dirfree) __P((char **, int));	/* DB_FUNC_DIRFREE */
	int	(*j_dirlist)				/* DB_FUNC_DIRLIST */
		    __P((const char *, char ***, int *));
	int	(*j_exists)				/* DB_FUNC_EXISTS */
		    __P((const char *, int *));
	void	(*j_free) __P((void *));		/* DB_FUNC_FREE */
	int	(*j_fsync) __P((int));			/* DB_FUNC_FSYNC */
	int	(*j_ioinfo) __P((const char *,		/* DB_FUNC_IOINFO */
		    int, u_int32_t *, u_int32_t *, u_int32_t *));
	void   *(*j_malloc) __P((size_t));		/* DB_FUNC_MALLOC */
	int	(*j_map)				/* DB_FUNC_MAP */
		    __P((char *, int, size_t, int, int, int, void **));
	int	(*j_open)				/* DB_FUNC_OPEN */
		    __P((const char *, int, ...));
	ssize_t	(*j_read) __P((int, void *, size_t));	/* DB_FUNC_READ */
	void   *(*j_realloc) __P((void *, size_t));	/* DB_FUNC_REALLOC */
	int	(*j_runlink) __P((char *));		/* DB_FUNC_RUNLINK */
	int	(*j_seek)				/* DB_FUNC_SEEK */
		    __P((int, size_t, db_pgno_t, u_int32_t, int, int));
	int	(*j_sleep) __P((u_long, u_long));	/* DB_FUNC_SLEEP */
	int	(*j_unlink) __P((const char *));	/* DB_FUNC_UNLINK */
	int	(*j_unmap) __P((void *, size_t));	/* DB_FUNC_UNMAP */
	ssize_t	(*j_write)				/* DB_FUNC_WRITE */
		    __P((int, const void *, size_t));
	int	(*j_yield) __P((void));			/* DB_FUNC_YIELD */
};

extern struct __db_jumptab __db_jump;
