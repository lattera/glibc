/* Get directory entries.  Linux/MIPSn64 LFS version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
#include <assert.h>
#include <sys/param.h>
#include <unistd.h>
#include <scratch_buffer.h>

ssize_t
__getdents64 (int fd, char *buf, size_t nbytes)
{
#ifdef __NR_getdents64
  ssize_t ret = INLINE_SYSCALL_CALL (getdents64, fd, buf, nbytes);
  if (ret != -1)
    return ret;
#endif

  /* Unfortunately getdents64 was only wire-up for MIPS n64 on Linux 3.10.
     If syscall is not available it need to fallback to non-LFS one.  */

  struct kernel_dirent
    {
      unsigned long d_ino;
      unsigned long d_off;
      unsigned short int d_reclen;
      char d_name[256];
    };

  const size_t size_diff = (offsetof (struct dirent64, d_name)
			   - offsetof (struct kernel_dirent, d_name));

  size_t red_nbytes = MIN (nbytes
			   - ((nbytes / (offsetof (struct dirent64, d_name)
					 + 14)) * size_diff),
			   nbytes - size_diff);

  struct scratch_buffer tmpbuf;
  scratch_buffer_init (&tmpbuf);
  if (!scratch_buffer_set_array_size (&tmpbuf, red_nbytes, sizeof (uint8_t)))
    INLINE_SYSCALL_ERROR_RETURN_VALUE (ENOMEM);

  struct kernel_dirent *skdp, *kdp;
  skdp = kdp = tmpbuf.data;

  ssize_t retval = INLINE_SYSCALL_CALL (getdents, fd, kdp, red_nbytes);
  if (retval == -1)
    {
      scratch_buffer_free (&tmpbuf);
      return -1;
    }

  off64_t last_offset = -1;
  struct dirent64 *dp = (struct dirent64 *) buf;
  while ((char *) kdp < (char *) skdp + retval)
    {
      const size_t alignment = _Alignof (struct dirent64);
      /* Since kdp->d_reclen is already aligned for the kernel structure
	 this may compute a value that is bigger than necessary.  */
      size_t new_reclen = ((kdp->d_reclen + size_diff + alignment - 1)
			   & ~(alignment - 1));
      if ((char *) dp + new_reclen > buf + nbytes)
        {
	  /* Our heuristic failed.  We read too many entries.  Reset
	     the stream.  */
	  assert (last_offset != -1);
	  __lseek64 (fd, last_offset, SEEK_SET);

	  if ((char *) dp == buf)
	    {
	      scratch_buffer_free (&tmpbuf);
	      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
	    }

	  break;
	}

      last_offset = kdp->d_off;
      dp->d_ino = kdp->d_ino;
      dp->d_off = kdp->d_off;
      dp->d_reclen = new_reclen;
      dp->d_type = *((char *) kdp + kdp->d_reclen - 1);
      memcpy (dp->d_name, kdp->d_name,
	      kdp->d_reclen - offsetof (struct kernel_dirent, d_name));

      dp = (struct dirent64 *) ((char *) dp + new_reclen);
      kdp = (struct kernel_dirent *) (((char *) kdp) + kdp->d_reclen);
    }

  scratch_buffer_free (&tmpbuf);
  return (char *) dp - buf;
}
#if _DIRENT_MATCHES_DIRENT64
strong_alias (__getdents64, __getdents)
#endif
