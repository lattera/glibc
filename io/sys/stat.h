/* Copyright (C) 1991, 92, 95, 96, 97, 98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 *	POSIX Standard: 5.6 File Characteristics	<sys/stat.h>
 */

#ifndef	_SYS_STAT_H
#define	_SYS_STAT_H	1

#include <features.h>

#include <bits/types.h>		/* For __mode_t and __dev_t.  */

#ifdef __USE_XOPEN
# define __need_time_t
# include <time.h>		/* For time_t.  */

/* The Single Unix specification says that some more types are
   available here.  */
# ifndef dev_t
typedef __dev_t dev_t;
#  define dev_t dev_t
# endif

# ifndef gid_t
typedef __gid_t gid_t;
#  define gid_t gid_t
# endif

# ifndef ino_t
#  ifndef __USE_FILE_OFFSET64
typedef __ino_t ino_t;
#  else
typedef __ino64_t ino_t;
#  endif
#  define ino_t ino_t
# endif

# ifndef mode_t
typedef __mode_t mode_t;
#  define mode_t mode_t
# endif

# ifndef nlink_t
typedef __nlink_t nlink_t;
#  define nlink_t nlink_t
# endif

# ifndef off_t
#  ifndef __USE_FILE_OFFSET64
typedef __off_t off_t;
#  else
typedef __off64_t off_t;
#  endif
#  define off_t off_t
# endif

# ifndef uid_t
typedef __uid_t uid_t;
#  define uid_t uid_t
# endif
#endif	/* X/Open */

#ifdef __USE_UNIX98
# ifndef pid_t
typedef __pid_t pid_t;
#  define pid_t pid_t
# endif
#endif	/* Unix98 */

__BEGIN_DECLS

#include <bits/stat.h>

#if defined __USE_BSD || defined __USE_MISC || defined __USE_XOPEN
# define S_IFMT		__S_IFMT
# define S_IFDIR	__S_IFDIR
# define S_IFCHR	__S_IFCHR
# define S_IFBLK	__S_IFBLK
# define S_IFREG	__S_IFREG
# ifdef __S_IFIFO
#  define S_IFIFO	__S_IFIFO
# endif
# if defined __USE_BSD || defined __USE_MISC
#  ifdef __S_IFLNK
#   define S_IFLNK	__S_IFLNK
#  endif
#  ifdef __S_IFSOCK
#   define S_IFSOCK	__S_IFSOCK
#  endif
# endif
#endif

/* Test macros for file types.	*/

#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

#define	S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)
#define	S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)
#define	S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)
#define	S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)
#ifdef __S_IFIFO
# define S_ISFIFO(mode)	 __S_ISTYPE((mode), __S_IFIFO)
#endif

#ifdef	__USE_BSD
# ifdef __S_IFLNK
#  define S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)
# else
#  define S_ISLNK(mode)  0
# endif
# ifdef __S_IFSOCK
#  define S_ISSOCK(mode) __S_ISTYPE((mode), __S_IFSOCK)
# endif
#endif


/* Protection bits.  */

#define	S_ISUID __S_ISUID	/* Set user ID on execution.  */
#define	S_ISGID	__S_ISGID	/* Set group ID on execution.  */

#if defined __USE_BSD || defined __USE_MISC
/* Save swapped text after use (sticky bit).  This is pretty well obsolete.  */
# define S_ISVTX	__S_ISVTX
#endif

#define	S_IRUSR	__S_IREAD	/* Read by owner.  */
#define	S_IWUSR	__S_IWRITE	/* Write by owner.  */
#define	S_IXUSR	__S_IEXEC	/* Execute by owner.  */
/* Read, write, and execute by owner.  */
#define	S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

#if defined __USE_MISC && defined __USE_BSD
# define S_IREAD		S_IRUSR
# define S_IWRITE	S_IWUSR
# define S_IEXEC		S_IXUSR
#endif

#define	S_IRGRP	(S_IRUSR >> 3)	/* Read by group.  */
#define	S_IWGRP	(S_IWUSR >> 3)	/* Write by group.  */
#define	S_IXGRP	(S_IXUSR >> 3)	/* Execute by group.  */
/* Read, write, and execute by group.  */
#define	S_IRWXG	(S_IRWXU >> 3)

#define	S_IROTH	(S_IRGRP >> 3)	/* Read by others.  */
#define	S_IWOTH	(S_IWGRP >> 3)	/* Write by others.  */
#define	S_IXOTH	(S_IXGRP >> 3)	/* Execute by others.  */
/* Read, write, and execute by others.  */
#define	S_IRWXO	(S_IRWXG >> 3)


#ifdef	__USE_BSD
/* Macros for common mode bit masks.  */
# define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
# define ALLPERMS (S_ISUID|S_ISGID|S_ISTXT|S_IRWXU|S_IRWXG|S_IRWXO)/* 07777 */
# define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)/* 0666*/

# define S_BLKSIZE	512	/* Block size for `st_blocks'.  */
#endif


#ifndef __USE_FILE_OFFSET64
/* Get file attributes for FILE and put them in BUF.  */
extern int stat __P ((__const char *__file, struct stat *__buf));

/* Get file attributes for the file, device, pipe, or socket
   that file descriptor FD is open on and put them in BUF.  */
extern int fstat __P ((int __fd, struct stat *__buf));
#else
# ifdef __REDIRECT
extern int __REDIRECT (stat, __P ((__const char *__file, struct stat *__buf)),
		       stat64);
extern int __REDIRECT (fstat, __P ((int __fd, struct stat *__buf)), fstat64);
# else
#  define stat stat64
#  define fstat fstat64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern int stat64 __P ((__const char *__file, struct stat64 *__buf));
extern int fstat64 __P ((int __fd, struct stat64 *__buf));
#endif

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
# ifndef __USE_FILE_OFFSET64
/* Get file attributes about FILE and put them in BUF.
   If FILE is a symbolic link, do not follow it.  */
extern int lstat __P ((__const char *__file, struct stat *__buf));
# else
#  ifdef __REDIRECT
extern int __REDIRECT (lstat, __P ((__const char *__file, struct stat *__buf)),
		       lstat64);
#  else
#   define lstat lstat64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int lstat64 __P ((__const char *__file, struct stat64 *__buf));
# endif
#endif

/* Set file access permissions for FILE to MODE.
   This takes an `int' MODE argument because that
   is what `mode_t's get widened to.  */
extern int chmod __P ((__const char *__file, __mode_t __mode));

/* Set file access permissions of the file FD is open on to MODE.  */
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern int fchmod __P ((int __fd, __mode_t __mode));
#endif


/* Set the file creation mask of the current process to MASK,
   and return the old creation mask.  */
extern __mode_t umask __P ((__mode_t __mask));

#ifdef	__USE_GNU
/* Get the current `umask' value without changing it.
   This function is only available under the GNU Hurd.  */
extern __mode_t getumask __P ((void));
#endif

/* Create a new directory named PATH, with permission bits MODE.  */
extern int mkdir __P ((__const char *__path, __mode_t __mode));

/* Create a device file named PATH, with permission and special bits MODE
   and device number DEV (which can be constructed from major and minor
   device numbers with the `makedev' macro above).  */
#if defined __USE_MISC || defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern int mknod __P ((__const char *__path,
		       __mode_t __mode, __dev_t __dev));
#endif


/* Create a new FIFO named PATH, with permission bits MODE.  */
extern int mkfifo __P ((__const char *__path, __mode_t __mode));

/* To allow the `struct stat' structure and the file type `mode_t'
   bits to vary without changing shared library major version number,
   the `stat' family of functions and `mknod' are in fact inline
   wrappers around calls to `xstat', `fxstat', `lxstat', and `xmknod',
   which all take a leading version-number argument designating the
   data structure and bits used.  <bits/stat.h> defines _STAT_VER with
   the version number corresponding to `struct stat' as defined in
   that file; and _MKNOD_VER with the version number corresponding to
   the S_IF* macros defined therein.  It is arranged that when not
   inlined these function are always statically linked; that way a
   dynamically-linked executable always encodes the version number
   corresponding to the data structures it uses, so the `x' functions
   in the shared library can adapt without needing to recompile all
   callers.  */

#ifndef _STAT_VER
# define _STAT_VER	0
#endif
#ifndef _MKNOD_VER
# define _MKNOD_VER	0
#endif

/* Wrappers for stat and mknod system calls.  */
#ifndef __USE_FILE_OFFSET64
extern int __fxstat __P ((int __ver, int __fildes,
			  struct stat *__stat_buf));
extern int __xstat __P ((int __ver, __const char *__filename,
			 struct stat *__stat_buf));
extern int __lxstat __P ((int __ver, __const char *__filename,
			  struct stat *__stat_buf));
#else
# ifdef __REDIRECT
extern int __REDIRECT (__fxstat, __P ((int __ver, int __fildes,
				       struct stat *__stat_buf)), __fxstat64);
extern int __REDIRECT (__xstat, __P ((int __ver, __const char *__filename,
				      struct stat *__stat_buf)), __xstat64);
extern int __REDIRECT (__lxstat, __P ((int __ver, __const char *__filename,
				       struct stat *__stat_buf)), __lxstat64);

# else
#  define __fxstat __fxstat64
#  define __xstat __xstat64
#  define __lxstat __lxstat64
# endif
#endif

#ifdef __USE_LARGEFILE64
extern int __fxstat64 __P ((int __ver, int __fildes,
			    struct stat64 *__stat_buf));
extern int __xstat64 __P ((int __ver, __const char *__filename,
			   struct stat64 *__stat_buf));
extern int __lxstat64 __P ((int __ver, __const char *__filename,
			    struct stat64 *__stat_buf));
#endif
extern int __xmknod __P ((int __ver, __const char *__path,
			  __mode_t __mode, __dev_t *__dev));

#if defined __GNUC__ && __GNUC__ >= 2
/* Inlined versions of the real stat and mknod functions.  */

extern __inline__ int stat (__const char *__path, struct stat *__statbuf)
{
  return __xstat (_STAT_VER, __path, __statbuf);
}

# if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern __inline__ int lstat (__const char *__path, struct stat *__statbuf)
{
  return __lxstat (_STAT_VER, __path, __statbuf);
}
# endif

extern __inline__ int fstat (int __fd, struct stat *__statbuf)
{
  return __fxstat (_STAT_VER, __fd, __statbuf);
}

# if defined __USE_MISC || defined __USE_BSD
extern __inline__ int mknod (__const char *__path, __mode_t __mode,
			     __dev_t __dev)
{
  return __xmknod (_MKNOD_VER, __path, __mode, &__dev);
}
# endif

# ifdef __USE_LARGEFILE64
extern __inline__ int stat64 (__const char *__path, struct stat64 *__statbuf)
{
  return __xstat64 (_STAT_VER, __path, __statbuf);
}

#  if defined __USE_BSD || defined __USE_XOPEN_EXTENDED
extern __inline__ int lstat64 (__const char *__path, struct stat64 *__statbuf)
{
  return __lxstat64 (_STAT_VER, __path, __statbuf);
}
#  endif

extern __inline__ int fstat64 (int __fd, struct stat64 *__statbuf)
{
  return __fxstat64 (_STAT_VER, __fd, __statbuf);
}
# endif

#endif

__END_DECLS


#endif /* sys/stat.h  */
