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
 *	POSIX Standard: 9.2.2 User Database Access	<pwd.h>
 */

#ifndef	_PWD_H
#define	_PWD_H	1

#include <features.h>

__BEGIN_DECLS

#include <bits/types.h>

#define __need_size_t
#include <stddef.h>

#ifdef __USE_XOPEN
/* The Single Unix specification says that some more types are
   available here.  */
# ifndef gid_t
typedef __gid_t gid_t;
#  define gid_t gid_t
# endif

# ifndef uid_t
typedef __uid_t uid_t;
#  define uid_t uid_t
# endif
#endif

/* The passwd structure.  */
struct passwd
{
  char *pw_name;		/* Username.  */
  char *pw_passwd;		/* Password.  */
  __uid_t pw_uid;		/* User ID.  */
  __gid_t pw_gid;		/* Group ID.  */
  char *pw_gecos;		/* Real name.  */
  char *pw_dir;			/* Home directory.  */
  char *pw_shell;		/* Shell program.  */
};


#if defined __USE_SVID || defined __USE_GNU
# define __need_FILE
# include <stdio.h>
#endif


#if defined __USE_SVID || defined __USE_MISC || defined __USE_XOPEN_EXTENDED
/* Rewind the password-file stream.  */
extern void setpwent __P ((void));

/* Close the password-file stream.  */
extern void endpwent __P ((void));

/* Read an entry from the password-file stream, opening it if necessary.  */
extern struct passwd *getpwent __P ((void));
#endif

#ifdef	__USE_SVID
/* Read an entry from STREAM.  */
extern struct passwd *fgetpwent __P ((FILE *__stream));

/* Write the given entry onto the given stream.  */
extern int putpwent __P ((__const struct passwd *__p, FILE *__f));
#endif

/* Search for an entry with a matching user ID.  */
extern struct passwd *getpwuid __P ((__uid_t __uid));

/* Search for an entry with a matching username.  */
extern struct passwd *getpwnam __P ((__const char *__name));

#if defined __USE_POSIX || defined __USE_MISC

# ifdef __USE_MISC
/* Reasonable value for the buffer sized used in the reentrant
   functions below.  But better use `sysconf'.  */
#  define NSS_BUFLEN_PASSWD	1024
# endif

/* Reentrant versions of some of the functions above.

   PLEASE NOTE: the `getpwent_r' function is not (yet) standardized.
   The interface may change in later versions of this library.  But
   the interface is designed following the principals used for the
   other reentrant functions so the chances are good this is what the
   POSIX people would choose.  */

# if defined __USE_SVID || defined __USE_MISC || defined __USE_XOPEN_EXTENDED
extern int getpwent_r __P ((struct passwd *__resultbuf, char *__buffer,
			    size_t __buflen, struct passwd **__result));
# endif

extern int getpwuid_r __P ((__uid_t __uid, struct passwd *__resultbuf,
			    char *__buffer, size_t __buflen,
			    struct passwd **__result));

extern int getpwnam_r __P ((__const char *__name, struct passwd *__resultbuf,
			    char *__buffer, size_t __buflen,
			    struct passwd **__result));


# ifdef	__USE_SVID
/* Read an entry from STREAM.  This function is not standardized and
   probably never will.  */
extern int fgetpwent_r __P ((FILE * __stream, struct passwd *__resultbuf,
			     char *__buffer, size_t __buflen,
			     struct passwd **__result));
# endif

#endif	/* POSIX or reentrant */


__END_DECLS

#endif /* pwd.h  */
