/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _STROPTS_H
#define _STROPTS_H	1

#include <features.h>
#include <bits/types.h>

/* Get system specific contants.  */
#include <bits/stropts.h>


__BEGIN_DECLS

/* Test whether FILDES is associated with a STREAM-based file.  */
extern int isastream __P ((int __fildes));

/* Receive next message from a STREAMS file.  */
extern int getmsg __P ((int __fildes, struct strbuf *__ctlptr,
			struct strbuf *__dataptr, int *__flagsp));

/* Receive next message from a STREAMS file, with *FLAGSP allowing to
   control which message.  */
extern int getpmsg __P ((int __fildes, struct strbuf *__ctlptr,
			 struct strbuf *__dataptr, int *__bandp,
			 int *__flagsp));

/* Perform the I/O control operation specified by REQUEST on FD.
   One argument may follow; its presence and type depend on REQUEST.
   Return value depends on REQUEST.  Usually -1 indicates error.  */
extern int ioctl __P ((int __fd, unsigned long int __request, ...));

/* Send a message on a STREAM.  */
extern int putmsg __P ((int __fildes, __const struct strbuf *__ctlptr,
			__const struct strbuf *__dataptr, int __flags));

/* Send a message on a STREAM to the BAND.  */
extern int putpmsg __P ((int __fildes, __const struct strbuf *__ctlptr,
			 __const struct strbuf *__dataptr, int __band,
			 int __flags));

/* Attach a STREAMS-based file descriptor FILDES to a file PATH in the
   file system name space.  */
extern int fattach __P ((int __fildes, __const char *__path));

/* Detach a name PATH from a STREAMS-based file descriptor.  */
extern int fdetach __P ((__const char *__path));

__END_DECLS

#endif /* stropts.h */
