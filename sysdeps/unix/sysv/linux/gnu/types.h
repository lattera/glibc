/* Copyright (C) 1991, 92, 94, 95, 96 Free Software Foundation, Inc.
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

#ifndef	_GNU_TYPES_H
#define	_GNU_TYPES_H	1

/* Get actual type definitions for architecture from kernel headers.
   This #define tells <linux/types.h> not to define `dev_t' et al itself.  */
#define _LINUX_TYPES_DONT_EXPORT
#include <linux/types.h>

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short __u_short;
typedef unsigned int __u_int;
typedef unsigned long __u_long;
#ifdef __GNUC__
typedef unsigned long long int __u_quad_t;
typedef long long int __quad_t;
typedef __quad_t *__qaddr_t;
#else
typedef struct
{
  long val[2];
} __quad_t;
typedef struct
{
  __u_long val[2];
} __u_quad_t;
#endif

typedef __kernel_dev_t __dev_t;		/* Type of device numbers.  */
typedef __kernel_uid_t __uid_t;		/* Type of user identifications.  */
typedef __kernel_gid_t __gid_t;		/* Type of group identifications.  */
typedef __kernel_ino_t __ino_t;		/* Type of file serial numbers.  */
typedef __kernel_mode_t __mode_t;	/* Type of file attribute bitmasks.  */
typedef __kernel_nlink_t __nlink_t; 	/* Type of file link counts.  */
typedef __kernel_off_t __off_t;		/* Type of file sizes and offsets.  */
typedef __kernel_pid_t __pid_t;		/* Type of process identifications.  */
typedef __kernel_ssize_t __ssize_t;	/* Type of a byte count, or error.  */
typedef __kernel_fsid_t __fsid_t;	/* Type of file system IDs.  */

/* Everythin' else.  */
typedef __kernel_daddr_t __daddr_t;	/* The type of a disk address.  */
typedef __kernel_caddr_t __caddr_t;
typedef __kernel_time_t __time_t;
typedef long int __swblk_t;		/* Type of a swap block maybe?  */

/* fd_set for select.  */
typedef __kernel_fd_set __fd_set;
typedef __kernel_clock_t __clock_t;

#endif /* gnu/types.h */
