/* Definitions for BSD-style memory management.  SunOS 4 version.
   Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifndef	_BITS_MMAN_H
#define	_BITS_MMAN_H	1

/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define	PROT_NONE	0x00	/* No access.  */
#define	PROT_READ	0x01	/* Pages can be read.  */
#define	PROT_WRITE	0x02	/* Pages can be written.  */
#define	PROT_EXEC	0x04	/* Pages can be executed.  */

/* Sharing types (must choose one and only one of these).  */
#define	MAP_SHARED	0x01	/* Share changes.  */
#define	MAP_PRIVATE	0x02	/* Changes private; copy pages on write.  */
#ifdef __USE_BSD
# define MAP_TYPE	0x0f	/* Mask for sharing type.  */
#endif

/* Other flags.  */
#define	MAP_FIXED	0x10	/* Map address must be exactly as requested. */
/* The following three flags are not actually implemented in SunOS 4.1.  */
#ifdef __USE_BSD
# define MAP_RENAME	0x20	/* Rename private pages to file.  */
# define MAP_NORESERVE	0x40	/* Don't reserve needed swap area.  */
# define MAP_INHERIT	0x80	/* Region is retained after exec.  */
#endif

/* This is an internal flag that is always set in `mmap' system calls.  In
   older versions of SunOS 4 `mmap' did not return the actual mapping
   address, but always returned zero.  This flag says to return the
   address; the `mmap' C library function always sets it.  */
#define	_MAP_NEW	0x80000000

/* Advice to `madvise'.  */
#ifdef __USE_BSD
# define MADV_NORMAL	0	/* No further special treatment.  */
# define MADV_RANDOM	1	/* Expect random page references.  */
# define MADV_SEQUENTIAL	2	/* Expect sequential page references.  */
# define MADV_WILLNEED	3	/* Will need these pages.  */
# define MADV_DONTNEED	4	/* Don't need these pages.  */
#endif

/* Flags to `msync'.  */
#define	MS_ASYNC	0x1		/* Return immediately, don't fsync.  */
#define	MS_INVALIDATE	0x2		/* Invalidate caches.  */

#endif /* bits/mman.h */
