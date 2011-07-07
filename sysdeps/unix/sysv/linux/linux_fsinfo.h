/* Constants from kernel header for various FSes.
   Copyright (C) 1998-2003,2005,2010,2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _LINUX_FSINFO_H
#define _LINUX_FSINFO_H	1

/* These definitions come from the kernel headers.  But we cannot
   include the headers here because of type clashes.  If new
   filesystem types will become available we have to add the
   appropriate definitions here.  */

/* Constant that identifies the `adfs' filesystem.  */
#define ADFS_SUPER_MAGIC	0xadf5

/* Constant that identifies the `affs' filesystem.  */
#define AFFS_SUPER_MAGIC	0xadff

/* Constant that identifies the `autofs' filesystem.  */
#define AUTOFS_SUPER_MAGIC	0x187

/* Constant that identifies the `bfs' filesystem.  */
#define BFS_MAGIC		0x1badface

/* Constant that identifies the `btrfs' filesystem.  */
#define BTRFS_SUPER_MAGIC	0x9123683e

/* Constant that identifies the `cgroup' filesystem.  */
#define CGROUP_SUPER_MAGIC	0x27e0eb

/* Constant that identifies the `coda' filesystem.  */
#define CODA_SUPER_MAGIC	0x73757245

/* Constant that identifies the `coherent' filesystem.  */
#define COH_SUPER_MAGIC		0x012ff7b7

/* Constant that identifies the `ramfs' filesystem.  */
#define CRAMFS_MAGIC		0x28cd3d45

/* Constant that identifies the `devfs' filesystem.  */
#define DEVFS_SUPER_MAGIC	0x1373

/* Constant that identifies the `devpts' filesystem.  */
#define DEVPTS_SUPER_MAGIC	0x1cd1

/* Constants that identifies the `efs' filesystem.  */
#define EFS_SUPER_MAGIC		0x414a53
#define EFS_MAGIC		0x072959

/* Constant that identifies the `ext2' and `ext3' filesystems.  */
#define EXT2_SUPER_MAGIC	0xef53

/* Constant that identifies the `hpfs' filesystem.  */
#define HPFS_SUPER_MAGIC	0xf995e849

/* Constant that identifies the `iso9660' filesystem.  */
#define ISOFS_SUPER_MAGIC	0x9660

/* Constant that identifies the `jffs' filesystem.  */
#define JFFS_SUPER_MAGIC	0x07c0

/* Constant that identifies the `jffs2' filesystem.  */
#define JFFS2_SUPER_MAGIC	0x72b6

/* Constant that identifies the `jfs' filesystem.  */
#define JFS_SUPER_MAGIC		0x3153464a

/* Constant that identifies the `logfs' filesystem.  */
#define LOGFS_MAGIC_U32		0xc97e8168u

/* Constant that identifies the `lustre' filesystem.  */
#define LUSTRE_SUPER_MAGIC	0x0BD00BD0

/* Constants that identify the `minix2' filesystem.  */
#define MINIX2_SUPER_MAGIC	0x2468
#define MINIX2_SUPER_MAGIC2	0x2478

/* Constants that identify the `minix' filesystem.  */
#define MINIX_SUPER_MAGIC	0x137f
#define MINIX_SUPER_MAGIC2	0x138F

/* Constant that identifies the `msdos' filesystem.  */
#define MSDOS_SUPER_MAGIC	0x4d44

/* Constant that identifies the `ncp' filesystem.  */
#define NCP_SUPER_MAGIC		0x564c

/* Constant that identifies the `nfs' filesystem.  */
#define NFS_SUPER_MAGIC		0x6969

/* Constant that identifies the `ntfs' filesystem.  */
#define NTFS_SUPER_MAGIC	0x5346544e

/* Constant that identifies the `proc' filesystem.  */
#define PROC_SUPER_MAGIC	0x9fa0

/* Constant that identifies the `usbdevfs' filesystem.  */
#define USBDEVFS_SUPER_MAGIC	0x9fa2

/* Constant that identifies the `qnx4' filesystem.  */
#define QNX4_SUPER_MAGIC	0x002f

/* Constant that identifies the `reiser' filesystem.  */
#define REISERFS_SUPER_MAGIC	0x52654973

/* Constant that identifies the `romfs' filesystem.  */
#define ROMFS_SUPER_MAGIC	0x7275

/* Constant that identifies the `shm' filesystem.  */
#define SHMFS_SUPER_MAGIC	0x01021994

/* Constant that identifies the `smb' filesystem.  */
#define SMB_SUPER_MAGIC		0x517b

/* Constant that identifies the `sysfs' filesystem.  */
#define SYSFS_MAGIC		0x62656572

/* Constants that identify the `sysV' filesystem.  */
#define SYSV2_SUPER_MAGIC	0x012ff7b6
#define SYSV4_SUPER_MAGIC	0x012ff7b5

/* Constant that identifies the `udf' filesystem.  */
#define UDF_SUPER_MAGIC		0x15013346

/* Constant that identify the `ufs' filesystem.  */
#define UFS_MAGIC		0x00011954
#define UFS_CIGAM		0x54190100 /* byteswapped MAGIC */

/* Constant that identifies the `vxfs' filesystem.  */
#define VXFS_SUPER_MAGIC	0xa501fcf5

/* Constant that identifies the `xenix' filesystem.  */
#define XENIX_SUPER_MAGIC	0x012ff7b4

/* Constant that identifies the `xfs' filesystem.  */
#define XFS_SUPER_MAGIC		0x58465342

/* Maximum link counts.  */
#define COH_LINK_MAX		10000
#define EXT2_LINK_MAX		32000
#define EXT4_LINK_MAX		65000
#define LUSTRE_LINK_MAX		EXT4_LINK_MAX
#define MINIX2_LINK_MAX		65530
#define MINIX_LINK_MAX		250
#define REISERFS_LINK_MAX	64535
#define SYSV_LINK_MAX		126     /* 127? 251? */
#define UFS_LINK_MAX		EXT2_LINK_MAX
#define XENIX_LINK_MAX		126     /* ?? */
#define XFS_LINK_MAX		2147483647

/* The Linux kernel header mentioned this as a kind of generic value.  */
#define LINUX_LINK_MAX	127


#endif	/* linux_fsinfo.h */
