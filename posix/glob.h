/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_GLOB_H

#define	_GLOB_H	1

#ifdef	__cplusplus
extern "C"
{
#endif

#undef	__ptr_t
#if (defined (__cplusplus) || (defined (__STDC__) && __STDC__) \
     || defined (WIN32))
#undef	__P
#define	__P(protos)	protos
#define	__ptr_t	void *
#else /* Not C++ or ANSI C.  */
#undef	__P
#define	__P(protos)	()
#undef	const
#define	const
#define	__ptr_t	char *
#endif /* C++ or ANSI C.  */

/* Bits set in the FLAGS argument to `glob'.  */
#define	GLOB_ERR	(1 << 0)/* Return on read errors.  */
#define	GLOB_MARK	(1 << 1)/* Append a slash to each name.  */
#define	GLOB_NOSORT	(1 << 2)/* Don't sort the names.  */
#define	GLOB_DOOFFS	(1 << 3)/* Insert PGLOB->gl_offs NULLs.  */
#define	GLOB_NOCHECK	(1 << 4)/* If nothing matches, return the pattern.  */
#define	GLOB_APPEND	(1 << 5)/* Append to results of a previous call.  */
#define	GLOB_NOESCAPE	(1 << 6)/* Backslashes don't quote metacharacters.  */
#define	GLOB_PERIOD	(1 << 7)/* Leading `.' can be matched by metachars.  */
#define	__GLOB_FLAGS	(GLOB_ERR|GLOB_MARK|GLOB_NOSORT|GLOB_DOOFFS| \
			 GLOB_NOESCAPE|GLOB_NOCHECK|GLOB_APPEND|     \
			 GLOB_PERIOD|GLOB_ALTDIRFUNC|GLOB_BRACE|     \
			 GLOB_NOMAGIC|GLOB_TILDE)

#if !defined (_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 2 || defined (_BSD_SOURCE)
#define	GLOB_MAGCHAR	(1 << 8)/* Set in gl_flags if any metachars seen.  */
#define GLOB_ALTDIRFUNC	(1 << 9)/* Use gl_opendir et al functions.  */
#define GLOB_BRACE	(1 << 10)/* Expand "{a,b}" to "a" "b".  */
#define GLOB_NOMAGIC	(1 << 11)/* If no magic chars, return the pattern.  */
#define GLOB_TILDE	(1 <<12)/* Expand ~user and ~ to home directories.  */
#endif

/* Error returns from `glob'.  */
#define	GLOB_NOSPACE	1	/* Ran out of memory.  */
#define	GLOB_ABEND	2	/* Read error.  */
#define	GLOB_NOMATCH	3	/* No matches found.  */

/* Structure describing a globbing run.  */
#if !defined (_AMIGA) && !defined (VMS) /* Buggy compiler.   */
struct stat;
#endif
typedef struct
  {
    int gl_pathc;		/* Count of paths matched by the pattern.  */
    char **gl_pathv;		/* List of matched pathnames.  */
    int gl_offs;		/* Slots to reserve in `gl_pathv'.  */
    int gl_flags;		/* Set to FLAGS, maybe | GLOB_MAGCHAR.  */

    /* If the GLOB_ALTDIRFUNC flag is set, the following functions
       are used instead of the normal file access functions.  */
    void (*gl_closedir) __P ((void *));
    struct dirent *(*gl_readdir) __P ((void *));
    __ptr_t (*gl_opendir) __P ((const char *));
    int (*gl_lstat) __P ((const char *, struct stat *));
    int (*gl_stat) __P ((const char *, struct stat *));
  } glob_t;

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   `errno' value from the failing call; if it returns non-zero
   `glob' returns GLOB_ABEND; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, `glob' returns zero.  */
extern int glob __P ((const char *__pattern, int __flags,
		      int (*__errfunc) __P ((const char *, int)),
		      glob_t *__pglob));

/* Free storage allocated in PGLOB by a previous `glob' call.  */
extern void globfree __P ((glob_t *__pglob));


#ifdef	__cplusplus
}
#endif

#endif /* glob.h  */
