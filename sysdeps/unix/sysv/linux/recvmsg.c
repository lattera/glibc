/* Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include <asm/posix_types.h>

/* The kernel expects this structure in SCM_CREDS messages.
 * Note: sizeof(struct __kernel_ucred) <= sizeof(struct cmsgcred) must hold.
 */
struct __kernel_ucred
{
  __kernel_pid_t pid;
  __kernel_uid_t uid;
  __kernel_gid_t gid;
};

extern int __syscall_recvmsg (int, struct msghdr *, int);

int
__libc_recvmsg (fd, message, flags)
     int fd;
     struct msghdr *message;
     int flags;
{
  struct cmsghdr *cm;
  int ret;
  int found_creds = 0;

  /* Must check for space first. */
  cm = CMSG_FIRSTHDR (message);
  while (cm)
    {
      if (cm->cmsg_type == SCM_CREDS)
	{
	  if (cm->cmsg_len < CMSG_SPACE (sizeof (struct cmsgcred)))
	    {
	      __set_errno (EINVAL);
	      return -1;
	    }
	  found_creds = 1;
	}
      cm = CMSG_NXTHDR (message, cm);
    }


  ret = __syscall_recvmsg (fd, message, flags);

  if (ret == -1)
    return ret;

  /* Postprocess the message control block for SCM_CREDS. */
  cm = CMSG_FIRSTHDR (message);
  if (found_creds)
    while (cm)
      {
	if (cm->cmsg_type == SCM_CREDS)
	  {
	    struct cmsgcred *c = (struct cmsgcred *) CMSG_DATA (cm);
	    struct __kernel_ucred u;
	    int i;
	    memcpy (&u, CMSG_DATA (cm), sizeof (struct __kernel_ucred));

	    c->cmcred_pid = u.pid;
	    c->cmcred_uid = u.uid;
	    c->cmcred_gid = u.gid;

	    c->cmcred_euid = -1;
	    c->cmcred_ngroups = 0;
	    for (i = 0; i < CMGROUP_MAX; i++)
	      c->cmcred_groups[i] = -1;
	  }
	cm = CMSG_NXTHDR (message, cm);
      }
  return ret;
}

weak_alias (__libc_recvmsg, __recvmsg)
weak_alias (__libc_recvmsg, recvmsg)
