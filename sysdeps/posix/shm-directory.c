/* Determine directory for shm/sem files.  Generic POSIX version.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#include <shm-directory.h>
#include <unistd.h>

#if _POSIX_MAPPED_FILES

# include <paths.h>

# define SHMDIR (_PATH_DEV "shm/")

const char *
__shm_directory (size_t *len)
{
  *len = sizeof SHMDIR - 1;
  return SHMDIR;
}
# if IS_IN (libpthread)
hidden_def (__shm_directory)
# endif

#endif
