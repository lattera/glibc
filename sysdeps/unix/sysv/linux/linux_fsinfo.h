/* Constants from kernel header for various FSes.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _LINUX_FSINFO_H
#define _LINUX_FSINFO_H	1

/* These definitions come from the kernel headers.  But we cannot
   include the headers here because of type clashes.  If new
   filesystem types will become available we have to add the
   appropriate definitions here.*/
#define ADFS_SUPER_MAGIC	0xadf5
#define AFFS_SUPER_MAGIC	0xadff
#define CODA_SUPER_MAGIC	0x73757245
#define EXT2_SUPER_MAGIC	0xef53
#define HPFS_SUPER_MAGIC	0xf995e849
#define ISOFS_SUPER_MAGIC	0x9660
#define MINIX_SUPER_MAGIC	0x137f
#define MINIX_SUPER_MAGIC2	0x138F
#define MINIX2_SUPER_MAGIC	0x2468
#define MINIX2_SUPER_MAGIC2	0x2478
#define MSDOS_SUPER_MAGIC	0x4d44
#define NCP_SUPER_MAGIC		0x564c
#define NFS_SUPER_MAGIC		0x6969
#define PROC_SUPER_MAGIC	0x9fa0
#define SMB_SUPER_MAGIC		0x517b
#define XENIX_SUPER_MAGIC	0x012ff7b4
#define SYSV4_SUPER_MAGIC	0x012ff7b5
#define SYSV2_SUPER_MAGIC	0x012ff7b6
#define COH_SUPER_MAGIC		0x012ff7b7
#define UFS_MAGIC		0x00011954
#define UFS_CIGAM		0x54190100 /* byteswapped MAGIC */

/* Maximum link counts.  */
#define EXT2_LINK_MAX		32000
#define MINIX_LINK_MAX		250
#define MINIX2_LINK_MAX		65530
#define XENIX_LINK_MAX		126     /* ?? */
#define SYSV_LINK_MAX		126     /* 127? 251? */
#define COH_LINK_MAX		10000
#define UFS_LINK_MAX		EXT2_LINK_MAX

#endif	/* linux_fsinfo.h */
