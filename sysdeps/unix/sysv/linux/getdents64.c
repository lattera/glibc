/* Get directory entries.  Linux LFS version.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

#include <string.h>
#include <dirent.h>
#include <errno.h>

/* The kernel struct linux_dirent64 matches the 'struct getdents64' type.  */
ssize_t
__getdents64 (int fd, char *buf, size_t nbytes)
{
  return INLINE_SYSCALL_CALL (getdents64, fd, buf, nbytes);
}

#if _DIRENT_MATCHES_DIRENT64
strong_alias (__getdents64, __getdents)
#else
# include <shlib-compat.h>

# if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
# include <olddirent.h>

/* kernel definition of as of 3.2.  */
struct compat_linux_dirent
{
  /* Both d_ino and d_off are compat_ulong_t which are defined in all
     architectures as 'u32'.  */
  uint32_t        d_ino;
  uint32_t        d_off;
  unsigned short  d_reclen;
  char            d_name[1];
};

ssize_t
__old_getdents64 (int fd, char *buf, size_t nbytes)
{
  ssize_t retval = INLINE_SYSCALL_CALL (getdents, fd, buf, nbytes);

  /* The kernel added the d_type value after the name.  Change this now.  */
  if (retval != -1)
    {
      union
      {
	struct compat_linux_dirent k;
	struct dirent u;
      } *kbuf = (void *) buf;

      while ((char *) kbuf < buf + retval)
	{
	  char d_type = *((char *) kbuf + kbuf->k.d_reclen - 1);
	  memmove (kbuf->u.d_name, kbuf->k.d_name,
		   strlen (kbuf->k.d_name) + 1);
	  kbuf->u.d_type = d_type;

	  kbuf = (void *) ((char *) kbuf + kbuf->k.d_reclen);
	}
     }
  return retval;
}
# endif /* SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)  */
#endif /* _DIRENT_MATCHES_DIRENT64  */
