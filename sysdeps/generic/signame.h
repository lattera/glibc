/* Convert between signal names and numbers.
   Copyright (C) 1990, 1992, 1993 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

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

/* Avoid conflicts with a system header file that might define these two.  */

#ifndef HAVE_PSIGNAL
/* Print to standard error the name of SIGNAL, preceded by MESSAGE and
   a colon, and followed by a newline.  */
void psignal (int signal, const char *message);
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
#if !defined (HAVE_SYS_SIGLIST)
extern char *sys_siglist[];
#endif

#endif
