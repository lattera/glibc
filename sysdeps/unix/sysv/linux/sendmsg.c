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
#include <unistd.h>

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

extern int __sendmsg (int, const struct msghdr *, int);

/* Send a message described by MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.  */
int
sendmsg (fd, message, flags)
     int fd;
     const struct msghdr *message;
     int flags;
{
  struct cmsghdr *cm;
  struct cmsgcred *cc;
  struct __kernel_ucred *u;
  pid_t pid;

  /* Preprocess the message control block for SCM_CREDS. */
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

	  u = (struct __kernel_ucred *) CMSG_DATA (cm);
	  cc = (struct cmsgcred *) CMSG_DATA (cm);
	  /* Linux expects the calling process to pass in
	     its credentials, and sanity checks them.
	     You can send real, effective, or set- uid and gid.
	     If the user hasn't filled in the buffer, we default to
	     real uid and gid. */
	  pid = getpid ();
	  if (cc->cmcred_pid != pid)
	  {
	      u->pid = pid;
	      u->uid = getuid ();
	      u->gid = getgid ();
	  }
	  else
	  {
	      struct __kernel_ucred v;
	      v.pid = cc->cmcred_pid;
	      v.uid = cc->cmcred_uid;
	      v.gid = cc->cmcred_gid;
	      u->pid = v.pid;
	      u->uid = v.uid;
	      u->gid = v.gid;
	  }
	}
      cm = CMSG_NXTHDR ((struct msghdr *) message, cm);
    }

  return __sendmsg (fd, message, flags);
}
