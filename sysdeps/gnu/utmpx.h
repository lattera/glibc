/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
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

#ifndef	_UTMPX_H
#define	_UTMPX_H	1

#include <features.h>
#include <sys/time.h>

/* Required according to Unix98.  */
#ifndef __pid_t_defined
typedef __pid_t pid_t;
# define __pid_t_defined
#endif

/* Get system dependent values and data structures.  */
#include <bits/utmpx.h>

#ifdef __USE_GNU
/* Compatibility names for the strings of the canonical file names.  */
# define UTMPX_FILE	_PATH_UTMPX
# define UTMPX_FILENAME	_PATH_UTMPX
# define WTMPX_FILE	_PATH_WTMPX
# define WTMPX_FILENAME	_PATH_WTMPX
#endif

/* For the getutmp{,x} functions we need the `struct utmp'.  */
#ifdef __USE_GNU
struct utmp;
#endif


__BEGIN_DECLS

/* Open user accounting database.  */
extern void setutxent (void) __THROW;

/* Close user accounting database.  */
extern void endutxent (void) __THROW;

/* Get the next entry from the user accounting database.  */
extern struct utmpx *getutxent (void) __THROW;

/* Get the user accounting database entry corresponding to ID.  */
extern struct utmpx *getutxid (__const struct utmpx *__id) __THROW;

/* Get the user accounting database entry corresponding to LINE.  */
extern struct utmpx *getutxline (__const struct utmpx *__line) __THROW;

/* Write the entry UTMPX into the user accounting database.  */
extern struct utmpx *pututxline (__const struct utmpx *__utmpx) __THROW;


#ifdef __USE_GNU
/* Change name of the utmpx file to be examined.  */
extern int utmpxname (__const char *__file) __THROW;

/* Append entry UTMP to the wtmpx-like file WTMPX_FILE.  */
extern void updwtmpx (__const char *__wtmpx_file,
		      __const struct utmpx *__utmpx) __THROW;


/* Copy the information in UTMPX to UTMP. */
extern void getutmp (__const struct utmpx *__utmpx,
		     struct utmp *__utmp) __THROW;

/* Copy the information in UTMP to UTMPX. */
extern void getutmpx (__const struct utmp *__utmp,
		      struct utmpx *__utmpx) __THROW;
#endif

__END_DECLS

#endif /* utmpx.h  */
