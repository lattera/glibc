/* Definitions for POSIX memory map interface.  Linux/HPPA version.
   Copyright (C) 1997, 1998, 2000, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman.h> directly; include <sys/mman.h> instead."
#endif

/* These are taken from the kernel definitions.  */

#define PROT_READ	0x1		/* Page can be read */
#define PROT_WRITE	0x2		/* Page can be written */
#define PROT_EXEC	0x4		/* Page can be executed */
#define PROT_NONE	0x0		/* Page can not be accessed */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of
					   growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of
					   growsup vma (mprotect only).  */

#define MAP_SHARED	0x01		/* Share changes */
#define MAP_PRIVATE	0x02		/* Changes are private */
#ifdef __USE_MISC
# define MAP_TYPE	0x03		/* Mask for type of mapping */
#endif

/* Other flags.  */
#define MAP_FIXED	0x04		/* Interpret addr exactly */
#ifdef __USE_MISC
# define MAP_FILE	0x0
# define MAP_ANONYMOUS	0x10		/* Don't use a file */
# define MAP_ANON	MAP_ANONYMOUS
# define MAP_VARIABLE	0
#endif

/* These are Linux-specific.  */
#ifdef __USE_MISC
# define MAP_DENYWRITE	0x0800		/* ETXTBSY */
# define MAP_EXECUTABLE	0x1000		/* Mark it as an executable */
# define MAP_LOCKED	0x2000		/* Pages are locked */
# define MAP_NORESERVE	0x4000		/* Don't check for reservations */
# define MAP_GROWSDOWN	0x8000		/* Stack-like segment */
# define MAP_POPULATE	0x10000		/* Populate (prefault) pagetables */
# define MAP_NONBLOCK	0x20000		/* Do not block on IO */
#endif

/* Flags to "msync"  */
#define MS_SYNC		1		/* Synchronous memory sync */
#define MS_ASYNC	2		/* Sync memory asynchronously */
#define MS_INVALIDATE	4		/* Invalidate the caches */

/* Flags to "mlockall"  */
#define MCL_CURRENT	1		/* Lock all current mappings */
#define MCL_FUTURE	2		/* Lock all future mappings */

/* Flags for `mremap'.  */
#ifdef __USE_GNU
# define MREMAP_MAYMOVE 1
# define MREMAP_FIXED	2
#endif

/* Advice to "madvise"  */
#ifdef __USE_BSD
# define MADV_NORMAL	  0	/* No further special treatment */
# define MADV_RANDOM	  1	/* Expect random page references */
# define MADV_SEQUENTIAL  2	/* Expect sequential page references */
# define MADV_WILLNEED	  3	/* Will need these pages */
# define MADV_DONTNEED	  4	/* Dont need these pages */
# define MADV_SPACEAVAIL  5	/* Insure that resources are reserved */
# define MADV_VPS_PURGE	  6	/* Purge pages from VM page cache */
# define MADV_VPS_INHERIT 7	/* Inherit parents page size */
# define MADV_REMOVE	  9	/* Remove these pages and resources.  */
# define MADV_DONTFORK	 10	/* Do not inherit across fork.  */
# define MADV_DOFORK	 11	/* Do inherit across fork.  */
# define MADV_MERGEABLE   65	/* KSM may merge identical pages */
# define MADV_UNMERGEABLE 66	/* KSM may not merge identical pages */
#endif

/* The range 12-64 is reserved for page size specification. */
#define MADV_4K_PAGES   12              /* Use 4K pages  */
#define MADV_16K_PAGES  14              /* Use 16K pages */
#define MADV_64K_PAGES  16              /* Use 64K pages */
#define MADV_256K_PAGES 18              /* Use 256K pages */
#define MADV_1M_PAGES   20              /* Use 1 Megabyte pages */
#define MADV_4M_PAGES   22              /* Use 4 Megabyte pages */
#define MADV_16M_PAGES  24              /* Use 16 Megabyte pages */
#define MADV_64M_PAGES  26              /* Use 64 Megabyte pages */

/* The POSIX people had to invent similar names for the same things.  */
#ifdef __USE_XOPEN2K
# define POSIX_MADV_NORMAL	0 /* No further special treatment.  */
# define POSIX_MADV_RANDOM	1 /* Expect random page references.  */
# define POSIX_MADV_SEQUENTIAL	2 /* Expect sequential page references.  */
# define POSIX_MADV_WILLNEED	3 /* Will need these pages.  */
# define POSIX_MADV_DONTNEED	4 /* Don't need these pages.  */
#endif
