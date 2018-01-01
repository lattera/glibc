/* Header for directory for shm/sem files.  NPTL version.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

#ifndef _SHM_DIRECTORY_H

#include <sysdeps/posix/shm-directory.h>

/* For NPTL the __shm_directory function lives in libpthread.
   We don't want PLT calls from there.  But it's also used from
   librt, so it cannot just be declared hidden.  */

#if IS_IN (libpthread)
hidden_proto (__shm_directory)
#endif

#endif  /* shm-directory.h */
