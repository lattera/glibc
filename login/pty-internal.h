/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

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

/* Internal constants used by the pseudoterminal handling code. */

#ifndef _PTY_INTERNAL_H
#define _PTY_INTERNAL_H	1

/* Length of a buffer to hold a pty name. */
#define PTYNAMELEN 15   /* "/dev/pts/65535$" */

/* Which group should pty slaves belong to: */
#define TTY_GROUP "tty"

/* Communication between grantpt and pt_chown. */
#define PTY_FD 3
enum  /* failure modes */
{
  FAIL_EBADF = 1,
  FAIL_EINVAL,
  FAIL_EACCES,
  FAIL_EXEC
};

#endif
