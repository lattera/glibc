/* Copyright (C) 1996 Free Software Foundation, Inc.
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

#ifndef	_SYS_KLOG_H

#define	_SYS_KLOG_H	1
#include <features.h>

/* Control the kernel's logging facility.  This corresponds exactly to
   the kernel's syslog system call, but that name is easily confused
   with the user-level syslog facility, which is something completely
   different.  */
extern int klogctl __P ((int __type, char *__bufp, int __len));

#endif /* _SYS_KLOG_H */
