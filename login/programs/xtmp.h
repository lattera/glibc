/* The `struct xtmp' type, describing the old linux utmp format.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#ifndef _XTMP_H
#define _XTMP_H		1

#include <sys/time.h>
#include <sys/types.h>


#define XT_LINESIZE	12
#define XT_NAMESIZE	8
#define XT_HOSTSIZE	16

struct xtmp
{
  short	int xt_type;		/* Type of login.  */
  pid_t	xt_pid;			/* Pid of login process.  */
  char	xt_line[XT_LINESIZE];	/* NUL-terminated devicename of tty.  */
  char	xt_id[4];		/* Inittab id.  */
  time_t xt_time;		/* Time entry was made.  */
  char	xt_user[XT_NAMESIZE];	/* Username (not NUL terminated).  */
  char	xt_host[XT_HOSTSIZE];	/* Hostname for remote login.  */
  long	xt_addr;		/* Internet address of remote host.  */
};

#define _HAVE_XT_TYPE	1
#define _HAVE_XT_PID	1
#define _HAVE_XT_ID	1
#define _HAVE_XT_HOST	1


extern void xtmp_to_utmp (const struct xtmp *xtmp, struct utmp *utmp);
extern void utmp_to_xtmp (const struct utmp *utmp, struct xtmp *xtmp);
extern int compare_entry (const struct utmp *xtmp,
			  const struct utmp *utmp);

#endif /* xtmp.h  */
