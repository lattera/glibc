/* Copyright (C) 1993, 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#ifndef	_UTMP_H_

#define	_UTMP_H_	1
#include <features.h>

#include <sys/types.h>


/* Get system dependent values and data structures.  */
#include <utmpbits.h>


/* Make FD be the controlling terminal, stdin, stdout, and stderr;
   then close FD.  Returns 0 on success, nonzero on error.  */
extern int login_tty __P ((int fd));


/* Write the given entry into utmp and wtmp.  */
extern void login __P ((__const struct utmp *));

/* Write the utmp entry to say the user on UT_LINE has logged out.  */
extern int logout __P ((__const char *ut_line));

/* Append to wtmp an entry for the current time and the given info.  */
extern void logwtmp __P ((__const char *ut_line, __const char *ut_name,
			  __const char *ut_host));

/* Change name of the utmp file to be examined.  */
extern int utmpname __P ((__const char *__file));

/* Read next entry from a utmp-like file.  */
extern struct utmp *getutent __P ((void));

/* Rest the input stream to the beginning of the file.  */
extern void setutent __P ((void));

/* Close the current open file.  */
extern void endutent __P ((void));

/* Search forward from the current point in the utmp file until the
   next entry with a ut_type matching ID->ut_type.  */
extern struct utmp *getutid __P ((__const struct utmp *__id));

/* Search forward from the current point in the utmp file until the
   next entry with a ut_line matching LINE->ut_line.  */
extern struct utmp *getutline __P ((__const struct utmp *__line));

/* Write out entry pointed to by UTMP_PTR into the utmp file.  */
extern struct utmp *pututline __P ((__const struct utmp *__utmp_ptr));


#ifdef	__USE_REENTRANT
/* Define the data structure needed for the reentrent version.  */
struct utmp_data
{
  int ut_fd;
  off_t loc_utmp;
  struct utmp ubuf;
};


/* Reentrent versions of the file for handling utmp files.  */
extern int getutent_r __P ((struct utmp **__utmp,
			    struct utmp_data *__utmp_data));

extern void setutent_r __P ((struct utmp_data *__utmp_data));

extern void endutent_r __P ((struct utmp_data *__utmp_data));

extern int getutid_r __P ((__const struct utmp *__id, struct utmp **__utmp,
			   struct utmp_data *__utmp_data));

extern int getutline_r __P ((__const struct utmp *__line, struct utmp **__utmp,
			     struct utmp_data *__utmp_data));

extern int pututline_r __P ((__const struct utmp *__utmp_ptr,
			     struct utmp_data *__utmp_data));

#endif	/* Use reentrant.  */

__END_DECLS

#endif /* utmp.h  */
