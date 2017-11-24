/* Memory-mapping-related declarations/definitions, not architecture-specific.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_MMAN_H
# error "Never use <bits/mman-shared.h> directly; include <sys/mman.h> instead."
#endif

#ifdef __USE_GNU
/* Flags for memfd_create.  */
# ifndef MFD_CLOEXEC
#  define MFD_CLOEXEC 1U
#  define MFD_ALLOW_SEALING 2U
#  define MFD_HUGETLB 4U
# endif

__BEGIN_DECLS

/* Create a new memory file descriptor.  NAME is a name for debugging.
   FLAGS is a combination of the MFD_* constants.  */
int memfd_create (const char *__name, unsigned int __flags) __THROW;

__END_DECLS

#endif /* __USE_GNU */
