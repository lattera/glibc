/* Copyright (C) 1991, 92, 94, 95, 96, 97 Free Software Foundation, Inc.
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

#ifndef	_GNU_TYPES_H
#define	_GNU_TYPES_H	1

#include <features.h>

/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short __u_short;
typedef unsigned int __u_int;
typedef unsigned long __u_long;
#ifdef __GNUC__
typedef unsigned long long int __u_quad_t;
typedef long long int __quad_t;
#else
typedef struct
  {
    long int __val[2];
  } __quad_t;
typedef struct
  {
    __u_long __val[2];
  } __u_quad_t;
#endif
typedef __quad_t *__qaddr_t;

typedef __u_long __dev_t;		/* Type of device numbers.  */
typedef __u_int __uid_t;		/* Type of user identifications.  */
typedef __u_int __gid_t;		/* Type of group identifications.  */
typedef __u_int __ino_t;		/* Type of file serial numbers.  */
typedef __u_int __mode_t;		/* Type of file attribute bitmasks.  */
typedef __u_int __nlink_t; 		/* Type of file link counts.  */
typedef long int __off_t;		/* Type of file sizes and offsets.  */
typedef __quad_t __loff_t;		/* Type of file sizes and offsets.  */
typedef int __pid_t;			/* Type of process identifications.  */
typedef long int __ssize_t;		/* Type of a byte count, or error.  */

typedef struct
  {
    int __val[2];
  } __fsid_t;				/* Type of file system IDs.  */

/* Everythin' else.  */
typedef int __daddr_t;			/* The type of a disk address.  */
typedef char *__caddr_t;
typedef long int __time_t;
typedef long int __swblk_t;		/* Type of a swap block maybe?  */

typedef long int __clock_t;

/* One element in the file descriptor mask array.  */
typedef unsigned int __fd_mask;

/* Due to incaution, we may have gotten these from a kernel header file.  */
#undef __FD_SETSIZE
#undef __NFDBITS
#undef __FDMASK

/* Number of descriptors that can fit in an `fd_set'.  */
#define __FD_SETSIZE	1024

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS	(8 * sizeof (__fd_mask))
#define	__FDELT(d)	((d) / __NFDBITS)
#define	__FDMASK(d)	(1 << ((d) % __NFDBITS))

/* fd_set for select and pselect.  */
typedef struct
  {
    /* XPG4.2 requires this member name.  */
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
  } __fd_set;


typedef int __key_t;

#endif /* gnu/types.h */
