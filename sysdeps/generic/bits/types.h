/* Copyright (C) 1991,1992,94-1999,2000,2001 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <sys/types.h> instead.
 */

#ifndef	_BITS_TYPES_H
#define	_BITS_TYPES_H	1

#include <features.h>

#define __need_size_t
#include <stddef.h>

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
  long __val[2];
} __quad_t;
typedef struct
{
  __u_long __val[2];
} __u_quad_t;
#endif
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#ifdef __GNUC__
typedef signed long long int __int64_t;
typedef unsigned long long int __uint64_t;
#endif
typedef __quad_t *__qaddr_t;

typedef int __dev_t;		/* Type of device numbers.  */
typedef unsigned int __uid_t;	/* Type of user identifications.  */
typedef unsigned int __gid_t;	/* Type of group identifications.  */
typedef unsigned int __ino_t;	/* Type of file serial numbers.  */
typedef __quad_t __ino64_t;	/* Type of file serial numbers (LFS).  */
typedef unsigned int __mode_t;	/* Type of file attribute bitmasks.  */
typedef unsigned short int __nlink_t; /* Type of file link counts.  */
typedef long int __off_t;	/* Type of file sizes and offsets.  */
typedef __quad_t __loff_t;	/* Type of file sizes and offsets.  */
typedef __loff_t __off64_t;	/* Type of file sizes and offsets (LFS).  */
typedef int __pid_t;		/* Type of process identifications.  */
typedef int __ssize_t;		/* Type of a byte count, or error.  */
typedef __u_quad_t __fsid_t;	/* Type of file system IDs.  */
typedef long int __clock_t;	/* Type of CPU usage counts.  */
typedef long int __rlim_t;	/* Type for resource measurement.  */
typedef __quad_t __rlim64_t;	/* Type for resource measurement (LFS).  */
typedef unsigned int __id_t;	/* General type for IDs.  */

/* Everythin' else.  */
typedef long int __daddr_t;	/* The type of a disk address.  */
typedef char *__caddr_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __swblk_t;	/* Type of a swap block maybe?  */
typedef long int __key_t;	/* Type of an IPC key */


/* Clock ID used in clock and timer functions.  */
typedef int __clockid_t;

/* Timer ID returned by `timer_create'.  */
typedef int __timer_t;


/* Number of descriptors that can fit in an `fd_set'.  */
#define	__FD_SETSIZE	256


/* XXX Used in `struct shmid_ds'.  */
typedef unsigned short int __ipc_pid_t;


/* Type to represent block size.  */
typedef unsigned int __blksize_t;

/* Types from the Large File Support interface.  */

/* Type to count number os disk blocks.  */
typedef long int __blkcnt_t;
typedef __quad_t __blkcnt64_t;

/* Type to count file system blocks.  */
typedef unsigned int __fsblkcnt_t;
typedef __u_quad_t __fsblkcnt64_t;

/* Type to count file system inodes.  */
typedef unsigned long int __fsfilcnt_t;
typedef __u_quad_t __fsfilcnt64_t;

/* Used in XTI.  */
typedef int __t_scalar_t;
typedef unsigned int __t_uscalar_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
typedef long int __intptr_t;

/* Duplicate info from sys/socket.h.  */
typedef unsigned int __socklen_t;

#endif /* bits/types.h */
