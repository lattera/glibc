/* Definitions for BSD-style memory management.  OSF/1 version.
Copyright (C) 1994 Free Software Foundation, Inc.
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

/* These are the bits used by 4.4 BSD and its derivatives.  On systems
   (such as GNU) where these facilities are not system services but can be
   emulated in the C library, these are the definitions we emulate.  */

#ifndef	_SYS_MMAN_H

#define	_SYS_MMAN_H	1
#include <features.h>

#include <gnu/types.h>
#define __need_size_t
#include <stddef.h>


/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define	PROT_NONE	0x00	/* No access.  */
#define	PROT_READ	0x01	/* Pages can be read.  */
#define	PROT_WRITE	0x02	/* Pages can be written.  */
#define	PROT_EXEC	0x04	/* Pages can be executed.  */


/* Flags contain mapping type, sharing type and options.  */

/* Mapping type (must choose one and only one of these).  */
#define	MAP_FILE	0x00	/* Mapped from a file or device.  */
#define	MAP_ANON	0x10	/* Allocated from anonymous virtual memory.  */
#define	MAP_ANONYMOUS	MAP_ANON
#define	MAP_TYPE	0xf0	/* Mask for type field.  */

/* Sharing types (must choose one and only one of these).  */
#define	MAP_SHARED	0x01	/* Share changes.  */
#define	MAP_PRIVATE	0x02	/* Changes private; copy pages on write.  */

/* Other flags.  */
#define	MAP_FIXED	0x0100	/* Map address must be exactly as requested. */
#define	MAP_VARIABLE	0	/* Absence of MAP_FIXED.  */
#define	MAP_HASSEMPHORE	0x0200	/* Region may contain semaphores.  */
#define	MAP_INHERIT	0x0400	/* Region is retained after exec.  */
#define	MAP_UNALIGNED	0x0800	/* File offset need not be page-aligned.  */

/* Advice to `madvise'.  */
#define	MADV_NORMAL	0	/* No further special treatment.  */
#define	MADV_RANDOM	1	/* Expect random page references.  */
#define	MADV_SEQUENTIAL	2	/* Expect sequential page references.  */
#define	MADV_WILLNEED	3	/* Will need these pages.  */
#define	MADV_DONTNEED	4	/* Don't need these pages.  */
#define	MADV_SPACEAVAIL	5	/* Ensure that resources are available.  */

/* Flags to `msync'.  */
#define MS_ASYNC	1		/* Asynchronous cache flush.  */
#define MS_SYNC		3		/* Synchronous cache flush.  */
#define MS_INVALIDATE	4		/* Invalidate cached pages.  */


#include <sys/cdefs.h>

__BEGIN_DECLS
/* Map addresses starting near ADDR and extending for LEN bytes.  from
   OFFSET into the file FD describes according to PROT and FLAGS.  If ADDR
   is nonzero, it is the desired mapping address.  If the MAP_FIXED bit is
   set in FLAGS, the mapping will be at ADDR exactly (which must be
   page-aligned); otherwise the system chooses a convenient nearby address.
   The return value is the actual mapping address chosen or (caddr_t) -1
   for errors (in which case `errno' is set).  A successful `mmap' call
   deallocates any previous mapping for the affected region.  */

__caddr_t mmap __P ((__caddr_t __addr, size_t __len,
		     int __prot, int __flags, int __fd, off_t __offset));

/* Deallocate any mapping for the region starting at ADDR and extending LEN
   bytes.  Returns 0 if successful, -1 for errors (and sets errno).  */
int munmap __P ((__caddr_t __addr, size_t __len));

/* Change the memory protection of the region starting at ADDR and
   extending LEN bytes to PROT.  Returns 0 if successful, -1 for errors
   (and sets errno).  */
int mprotect __P ((__caddr_t __addr, size_t __len, int __prot));

/* Synchronize the region starting at ADDR and extending LEN bytes with the
   file it maps.  Filesystem operations on a file being mapped are
   unpredictable before this is done.  */
int msync __P ((__caddr_t __addr, size_t __len, int __flags));

/* Advise the system about particular usage patterns the program follows
   for the region starting at ADDR and extending LEN bytes.  */
int madvise __P ((__caddr_t __addr, size_t __len, int __advice));

__END_DECLS


#endif	/* sys/mman.h */
