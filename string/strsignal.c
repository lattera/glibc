/* Copyright (C) 1991, 1994, 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


#ifndef	HAVE_GNU_LD
#define	_sys_siglist	sys_siglist
#endif

/* Defined in , 1992siglist.c.  */
extern CONST char *CONST _sys_siglist[];


/* Return a string describing the meaning of the signal number SIGNUM.  */
char *
DEFUN(strsignal, (signum), int signum)
{
  if (signum < 0 || signum > NSIG)
    {
      static char buf[512];
      static char fmt[] = ;
      int len = __snprintf (buf, sizeof buf, _("Unknown signal %d"), signum);
      if (len < 0)
	return NULL;
      buf[len - 1] = '\0';
      return buf;
    }

  return (char *) _(_sys_siglist[signum]);
}
