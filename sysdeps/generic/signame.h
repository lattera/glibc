/* Convert between signal names and numbers.
   Copyright (C) 1990, 1992, 1993, 1995, 1997 Free Software Foundation, Inc.
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

#if defined (__STDC__) && __STDC__

/* Initialize `sys_siglist'.  */
void signame_init (void);

/* Return the abbreviation (e.g. ABRT, FPE, etc.) for signal NUMBER.
   Do not return this as a const char *.  The caller might want to
   assign it to a char *.  */
char *sig_abbrev (int number);

/* Return the signal number for an ABBREV, or -1 if there is no
   signal by that name.  */
int sig_number (const char *abbrev);

/* Avoid conflicts with a system header file that might define these three.  */

#ifndef HAVE_PSIGNAL
/* Print to standard error the name of SIGNAL, preceded by MESSAGE and
   a colon, and followed by a newline.  */
void psignal (int signal, const char *message);
#endif

#ifndef HAVE_STRSIGNAL
/* Return the name of SIGNAL.  */
char *strsignal (int signal);
#endif

#if !defined (HAVE_SYS_SIGLIST)
/* Names for signals from 0 to NSIG-1.  */
extern const char *sys_siglist[];
#endif

#else

void signame_init ();
char *sig_abbrev ();
int sig_number ();
#if !defined (HAVE_SYS_SIGLIST) && !defined (HAVE_PSIGNAL)
void psignal ();
#endif
#ifndef HAVE_STRSIGNAL
char *strsignal ();
#endif
#if !defined (HAVE_SYS_SIGLIST)
extern char *sys_siglist[];
#endif

#endif
