/* Definitions for POSIX-style memory management.  Linux version.
   Copyright (C) 1994, 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifndef	_SYS_MMAN_H
#define	_SYS_MMAN_H	1

#include <features.h>
#include <bits/types.h>
#define __need_size_t
#include <stddef.h>

/* Get the bit values from the kernel header file.  */
#include <bits/mman.h>

/* Return value of `mmap' in case of an error.  */
#define MAP_FAILED	((__ptr_t) -1)

__BEGIN_DECLS
/* Map addresses starting near ADDR and extending for LEN bytes.  from
   OFFSET into the file FD describes according to PROT and FLAGS.  If ADDR
   is nonzero, it is the desired mapping address.  If the MAP_FIXED bit is
   set in FLAGS, the mapping will be at ADDR exactly (which must be
   page-aligned); otherwise the system chooses a convenient nearby address.
   The return value is the actual mapping address chosen or MAP_FAILED
   for errors (in which case `errno' is set).  A successful `mmap' call
   deallocates any previous mapping for the affected region.  */

#ifndef __USE_FILE_OFFSET64
extern __ptr_t mmap __P ((__ptr_t __addr, size_t __len, int __prot,
			  int __flags, int __fd, __off_t __offset));
#else
extern __ptr_t mmap __P ((__ptr_t __addr, size_t __len, int __prot,
			  int __flags, int __fd, __off64_t __offset))
     __asm__ ("mmap64");
#endif
#ifdef __USE_LARGEFILE64
extern __ptr_t mmap64 __P ((__ptr_t __addr, size_t __len, int __prot,
			    int __flags, int __fd, __off64_t __offset));
#endif

/* Deallocate any mapping for the region starting at ADDR and extending LEN
   bytes.  Returns 0 if successful, -1 for errors (and sets errno).  */
extern int munmap __P ((__ptr_t __addr, size_t __len));

/* Change the memory protection of the region starting at ADDR and
   extending LEN bytes to PROT.  Returns 0 if successful, -1 for errors
   (and sets errno).  */
extern int mprotect __P ((__ptr_t __addr, size_t __len, int __prot));

/* Synchronize the region starting at ADDR and extending LEN bytes with the
   file it maps.  Filesystem operations on a file being mapped are
   unpredictable before this is done.  Flags are from the MS_* set.  */
extern int msync __P ((__ptr_t __addr, size_t __len, int __flags));

#ifdef __USE_BSD
/* Advise the system about particular usage patterns the program follows
   for the region starting at ADDR and extending LEN bytes.  */
extern int madvise __P ((__ptr_t __addr, size_t __len, int __advice));
#endif

/* Cause all currently mapped pages of the process to be memory resident
   until unlocked by a call to the `munlockall', until the process exits,
   or until the process calls `execve'.  */
extern int mlockall __P ((int __flags));

/* All currently mapped pages of the process' address space become
   unlocked.  */
extern int munlockall __P ((void));

/* Guarantee all whole pages mapped by the range [ADDR,ADDR+LEN) to
   be memory resident.  */
extern int mlock __P ((__const __ptr_t __addr, size_t __len));

/* Unlock whole pages previously mapped by the range [ADDR,ADDR+LEN).  */
extern int munlock __P ((__const __ptr_t __addr, size_t __len));

#ifdef __USE_MISC
/* Remap pages mapped by the range [ADDR,ADDR+OLD_LEN) to new length
   NEW_LEN.  If MAY_MOVE is MREMAP_MAYMOVE the returned address may
   differ from ADDR.  */
extern __ptr_t mremap __P ((__ptr_t __addr, size_t __old_len,
			  size_t __new_len, int __may_move));
#endif

__END_DECLS

#endif	/* _SYS_MMAN_H */
