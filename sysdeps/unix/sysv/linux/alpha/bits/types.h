/* Copyright (C) 1991, 92, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;
typedef unsigned long int __u_quad_t;
typedef long int __quad_t;
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
typedef __quad_t *__qaddr_t;

typedef __uint64_t __dev_t;		/* Type of device numbers.  */
typedef __uint32_t __uid_t;		/* Type of user identifications.  */
typedef __uint32_t __gid_t;		/* Type of group identifications.  */
typedef __uint32_t __ino_t;		/* Type of file serial numbers.  */
typedef __uint64_t __ino64_t;		/*  "" (LFS) */
typedef __uint32_t __mode_t;		/* Type of file attribute bitmasks.  */
typedef __uint32_t __nlink_t; 		/* Type of file link counts.  */
typedef __int64_t  __off_t;		/* Type of file sizes and offsets.  */
typedef __int64_t  __off64_t;		/*  "" (LFS) */
typedef __int64_t  __loff_t;		/* Type of file sizes and offsets.  */
typedef __int32_t  __pid_t;		/* Type of process identifications.  */
typedef __int64_t  __ssize_t;		/* Type of a byte count, or error.  */
typedef __int64_t  __rlim_t;		/* Type of resource counts.  */
typedef __int64_t  __rlim64_t;		/*  "" (LFS) */
typedef __int32_t  __blkcnt_t;		/* Type to count nr disk blocks.  */
typedef __int64_t  __blkcnt64_t;	/*  "" (LFS) */
typedef __uint32_t __fsblkcnt_t;	/* Type to count file system blocks.  */
typedef __uint64_t __fsblkcnt64_t;	/*  "" (LFS) */
typedef __uint64_t __fsfilcnt_t;	/* Type to count file system inodes.  */
typedef __uint64_t __fsfilcnt64_t;	/*  "" (LFS) */
typedef __uint32_t __id_t;		/* General type for IDs.  */

typedef struct
  {
    int __val[2];
  } __fsid_t;				/* Type of file system IDs.  */

/* Everythin' else.  */
typedef int __daddr_t;			/* Type of a disk address.  */
typedef char *__caddr_t;		/* Type of a core address.  */
typedef long int __time_t;
typedef long int __swblk_t;		/* Type of a swap block maybe?  */
typedef long int __clock_t;
typedef int __key_t;			/* Type of a SYSV IPC key. */

/* Used in `struct shmid_ds'.  */
typedef int __ipc_pid_t;

/* One element in the file descriptor mask array.  */
typedef unsigned long int __fd_mask;

/* Due to incaution, we may have gotten these from a kernel header file.  */
#undef __FD_SETSIZE
#undef __NFDBITS
#undef __FDMASK

/* Number of descriptors that can fit in an `fd_set'.  */
#define __FD_SETSIZE	1024

/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS	(8 * sizeof (__fd_mask))
#define	__FDELT(d)	((d) / __NFDBITS)
#define	__FDMASK(d)	((__fd_mask) 1 << ((d) % __NFDBITS))

/* fd_set for select and pselect.  */
typedef struct
  {
    /* XPG4.2 requires this member name.  Otherwise avoid the name
       from the user namespace.  */
#ifdef __USE_XOPEN
    __fd_mask fds_bits[__FD_SETSIZE / __NFDBITS];
#else
    __fd_mask __fds_bits[__FD_SETSIZE / __NFDBITS];
#endif
  } __fd_set;

/* Used in XTI.  */
typedef int __t_scalar_t;
typedef unsigned int __t_uscalar_t;

/* Duplicates info from stdint.h but this is used in unistd.h.  */
typedef long int __intptr_t;


/* Now add the thread types.  */
#ifdef __USE_UNIX98
# include <bits/pthreadtypes.h>
#endif

#endif /* bits/types.h */
