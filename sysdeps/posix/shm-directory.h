/* Header for directory for shm/sem files.
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

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern const char *__shm_directory (size_t *len);

/* This defines local variables SHM_DIR and SHM_DIRLEN, giving the
   directory prefix (with trailing slash) and length (not including '\0'
   terminator) of the directory used for shm files.  If that cannot be
   determined, it sets errno to ENOSYS and returns RETVAL_FOR_INVALID.

   This uses the local variable NAME as an lvalue, and increments it past
   any leading slashes.  It then defines the local variable NAMELEN, giving
   strlen (NAME) + 1.  If NAME is invalid, it sets errno to
   ERRNO_FOR_INVALID and returns RETVAL_FOR_INVALID.  Finally, it defines
   the local variable SHM_NAME, giving the absolute file name of the shm
   file corresponding to NAME.  PREFIX is a string constant used as a
   prefix on NAME.  */

#define SHM_GET_NAME(errno_for_invalid, retval_for_invalid, prefix)           \
  size_t shm_dirlen;							      \
  const char *shm_dir = __shm_directory (&shm_dirlen);			      \
  /* If we don't know what directory to use, there is nothing we can do.  */  \
  if (__glibc_unlikely (shm_dir == NULL))				      \
    {									      \
      __set_errno (ENOSYS);						      \
      return retval_for_invalid;					      \
    }									      \
  /* Construct the filename.  */					      \
  while (name[0] == '/')						      \
    ++name;								      \
  size_t namelen = strlen (name) + 1;					      \
  /* Validate the filename.  */						      \
  if (namelen == 1 || namelen >= NAME_MAX || strchr (name, '/') != NULL)      \
    {									      \
      __set_errno (errno_for_invalid);					      \
      return retval_for_invalid;					      \
    }									      \
  char *shm_name = __alloca (shm_dirlen + sizeof prefix - 1 + namelen);	      \
  __mempcpy (__mempcpy (__mempcpy (shm_name, shm_dir, shm_dirlen),	      \
                        prefix, sizeof prefix - 1),			      \
             name, namelen)

#endif	/* shm-directory.h */
