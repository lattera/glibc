/* Flags for BSD-style memory management.  OSF/1 version.
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

#define	PROT_NONE	 0x00	/* No access.  */
#define	PROT_READ	 0x01	/* Pages can be read.  */
#define	PROT_WRITE	 0x02	/* Pages can be written.  */
#define	PROT_EXEC	 0x04	/* Pages can be executed.  */

/* Flags contain mapping type, sharing type and options.  */

/* Mapping type (must choose one and only one of these).  */
#ifdef __USE_BSD
# define MAP_FILE	 0x00	/* Mapped from a file or device.  */
# define MAP_ANON	 0x10	/* Allocated from anonymous virtual memory.  */
# define MAP_ANONYMOUS	 MAP_ANON
# define MAP_TYPE	 0xf0	/* Mask for type field.  */
#endif

/* Sharing types (must choose one and only one of these).  */
#define	MAP_SHARED	 0x01	/* Share changes.  */
#define	MAP_PRIVATE	 0x02	/* Changes private; copy pages on write.  */

/* Other flags.  */
#define	MAP_FIXED	 0x0100	/* Map address must be exactly as requested. */
#ifdef __USE_BSD
# define MAP_VARIABLE	 0	/* Absence of MAP_FIXED.  */
# define MAP_HASSEMPHORE 0x0200	/* Region may contain semaphores.  */
# define MAP_INHERIT	 0x0400	/* Region is retained after exec.  */
# define MAP_UNALIGNED	 0x0800	/* File offset need not be page-aligned.  */
#endif

/* Advice to `madvise'.  */
#ifdef __USE_BSD
# define MADV_NORMAL	 0	/* No further special treatment.  */
# define MADV_RANDOM	 1	/* Expect random page references.  */
# define MADV_SEQUENTIAL 2	/* Expect sequential page references.  */
# define MADV_WILLNEED	 3	/* Will need these pages.  */
# define MADV_DONTNEED	 4	/* Don't need these pages.  */
# define MADV_SPACEAVAIL 5	/* Ensure that resources are available.  */
#endif

/* Flags to `msync'.  */
#define MS_ASYNC	1		/* Asynchronous cache flush.  */
#define MS_SYNC		3		/* Synchronous cache flush.  */
#define MS_INVALIDATE	4		/* Invalidate cached pages.  */

#endif /* bits/mman.h */
