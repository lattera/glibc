/* Copyright (C) 1993, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifndef	_UTMP_H
#define	_UTMP_H	1

#include <features.h>

#include <sys/types.h>


__BEGIN_DECLS

/* Get system dependent values and data structures.  */
#include <bits/utmp.h>

/* Compatibility names for the strings of the canonical file names.  */
#define UTMP_FILE	_PATH_UTMP
#define UTMP_FILENAME	_PATH_UTMP
#define WTMP_FILE	_PATH_WTMP
#define WTMP_FILENAME	_PATH_WTMP



/* Make FD be the controlling terminal, stdin, stdout, and stderr;
   then close FD.  Returns 0 on success, nonzero on error.  */
extern int login_tty __P ((int __fd));


/* Write the given entry into utmp and wtmp.  */
extern void login __P ((__const struct utmp *__entry));

/* Write the utmp entry to say the user on UT_LINE has logged out.  */
extern int logout __P ((__const char *__ut_line));

/* Append to wtmp an entry for the current time and the given info.  */
extern void logwtmp __P ((__const char *__ut_line, __const char *__ut_name,
			  __const char *__ut_host));

/* Append entry UTMP to the wtmp-like file WTMP_FILE.  */
extern void __updwtmp __P ((__const char *__wtmp_file,
			    __const struct utmp *__utmp));
extern void updwtmp __P ((__const char *__wtmp_file,
			  __const struct utmp *__utmp));

/* Change name of the utmp file to be examined.  */
extern int __utmpname __P ((__const char *__file));
extern int utmpname __P ((__const char *__file));

/* Read next entry from a utmp-like file.  */
extern struct utmp *__getutent __P ((void));
extern struct utmp *getutent __P ((void));

/* Reset the input stream to the beginning of the file.  */
extern void __setutent __P ((void));
extern void setutent __P ((void));

/* Close the current open file.  */
extern void __endutent __P ((void));
extern void endutent __P ((void));

/* Search forward from the current point in the utmp file until the
   next entry with a ut_type matching ID->ut_type.  */
extern struct utmp *__getutid __P ((__const struct utmp *__id));
extern struct utmp *getutid __P ((__const struct utmp *__id));

/* Search forward from the current point in the utmp file until the
   next entry with a ut_line matching LINE->ut_line.  */
extern struct utmp *__getutline __P ((__const struct utmp *__line));
extern struct utmp *getutline __P ((__const struct utmp *__line));

/* Write out entry pointed to by UTMP_PTR into the utmp file.  */
extern struct utmp *__pututline __P ((__const struct utmp *__utmp_ptr));
extern struct utmp *pututline __P ((__const struct utmp *__utmp_ptr));


#ifdef	__USE_MISC
/* Reentrant versions of the file for handling utmp files.  */
extern int __getutent_r __P ((struct utmp *__buffer, struct utmp **__result));
extern int getutent_r __P ((struct utmp *__buffer, struct utmp **__result));

extern int __getutid_r __P ((__const struct utmp *__id, struct utmp *__buffer,
			     struct utmp **__result));
extern int getutid_r __P ((__const struct utmp *__id, struct utmp *__buffer,
			   struct utmp **__result));

extern int __getutline_r __P ((__const struct utmp *__line,
			       struct utmp *__buffer, struct utmp **__result));
extern int getutline_r __P ((__const struct utmp *__line,
			     struct utmp *__buffer, struct utmp **__result));

#endif	/* Use misc.  */

__END_DECLS

#endif /* utmp.h  */
