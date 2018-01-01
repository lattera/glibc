/* shm_open -- open a POSIX shared memory object.  Generic POSIX file version.
   Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

#include <unistd.h>

#if ! _POSIX_MAPPED_FILES

# include <rt/shm_open.c>

#else

# include <fcntl.h>
# include <pthread.h>
# include <shm-directory.h>


/* Open shared memory object.  */
int
shm_open (const char *name, int oflag, mode_t mode)
{
  SHM_GET_NAME (EINVAL, -1, "");

  oflag |= O_NOFOLLOW | O_CLOEXEC;

  /* Disable asynchronous cancellation.  */
  int state;
  pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &state);

  int fd = open (shm_name, oflag, mode);
  if (fd == -1 && __glibc_unlikely (errno == EISDIR))
    /* It might be better to fold this error with EINVAL since
       directory names are just another example for unsuitable shared
       object names and the standard does not mention EISDIR.  */
    __set_errno (EINVAL);

  pthread_setcancelstate (state, NULL);

  return fd;
}

#endif  /* _POSIX_MAPPED_FILES */
