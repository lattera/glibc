/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 9.2.2 User Database Access	<pwd.h>
 */

#ifndef	_PWD_H

#define	_PWD_H	1
#include <features.h>

__BEGIN_DECLS

#include <gnu/types.h>


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


#if defined(__USE_SVID) || defined(__USE_GNU)
#define	__need_FILE
#include <stdio.h>
#endif

#ifdef	__USE_GNU
/* Return a new stream open on the password file.  */
extern FILE *__pwdopen __P ((void));

/* Read a password entry from STREAM, filling in P.
   Return the `struct passwd' of P if successful, NULL on failure.  */
extern struct passwd *__pwdread __P ((FILE * __stream, __ptr_t __p));

/* Return a chunk of memory containing pre-initialized data for __pwdread.  */
extern __ptr_t __pwdalloc __P ((void));
#endif


#if defined(__USE_SVID) || defined(__USE_MISC)
/* Rewind the password-file stream.  */
extern void setpwent __P ((void));

/* Close the password-file stream.  */
extern void endpwent __P ((void));

/* Read an entry from the password-file stream, opening it if necessary.  */
extern struct passwd *getpwent __P ((void));
#endif

#ifdef	__USE_SVID
/* Read an entry from STREAM.  */
extern struct passwd *fgetpwent __P ((FILE * __stream));

/* Write the given entry onto the given stream.  */
extern int putpwent __P ((__const struct passwd * __p, FILE * __f));
#endif

/* Search for an entry with a matching user ID.  */
extern struct passwd *getpwuid __P ((__uid_t __uid));

/* Search for an entry with a matching username.  */
extern struct passwd *getpwnam __P ((__const char *__name));


__END_DECLS

#endif /* pwd.h  */
