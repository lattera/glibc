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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <asm/posix_types.h>

/* The kernel expects this structure in SCM_CREDS messages.
 * Note: sizeof(struct __kernel_ucred) <= sizeof(struct cmsgcred) must hold.
 */
struct kernel_ucred
  {
    __kernel_pid_t pid;
    __kernel_uid_t uid;
    __kernel_gid_t gid;
  };

struct credmsg
  {
    struct cmsghdr cm;
    struct cmsgcred cc;
  };

struct kcredmsg
  {
    struct cmsghdr cm;
    struct kernel_ucred cc;
  };

extern int __syscall_sendmsg (int, const struct msghdr *, int);

/* Send a message described by MESSAGE on socket FD.
   Returns the number of bytes sent, or -1 for errors.  */
int
__libc_sendmsg (int fd, const struct msghdr *message, int flags)
{
  struct msghdr m;
  char *buf, *a, *b;
  struct credmsg *cred = 0;
  struct kcredmsg *kcred;
  struct cmsghdr *cm;
  long int offset = 0;
  pid_t pid;

  /* Preprocess the message control block for SCM_CREDS. */
  if (message->msg_controllen)
    {
      cm = CMSG_FIRSTHDR (message);
      while (cm)
	{
	  if (cm->cmsg_type == SCM_CREDS)
	    {
	      if (cred ||
		  cm->cmsg_len < CMSG_LEN (sizeof (struct cmsgcred)))
		{
		  __set_errno (EINVAL);
		  return -1;
		}
	      else
		{
		  cred = (struct credmsg *) cm;
		  offset = (char *) cm - (char *) message->msg_control;
		}
	    }
	  cm = CMSG_NXTHDR ((struct msghdr *) message, cm);
	}

      if (cred)
	{
	  buf = alloca (message->msg_controllen);
	  memcpy (buf, message->msg_control, message->msg_controllen);
	  kcred = (struct kcredmsg *) (buf + offset);
	  a = (char *) kcred + CMSG_LEN (sizeof (struct kernel_ucred));
	  b = (char *) kcred + CMSG_LEN (sizeof (struct cmsgcred));
	  memmove (a, b, message->msg_controllen - (b - buf));

	  kcred->cm.cmsg_len = CMSG_LEN (sizeof (struct kernel_ucred));

	  /* Linux expects the calling process to pass in
	     its credentials, and sanity checks them.
	     You can send real, effective, or set- uid and gid.
	     If the user hasn't filled in the buffer, we default to
	     real uid and gid. */
	  pid = __getpid ();
	  if (cred->cc.cmcred_pid != pid)
	    {
	      kcred->cc.pid = pid;
	      kcred->cc.uid = __getuid ();
	      kcred->cc.gid = __getgid ();
	    }
	  else
	    {
	      kcred->cc.uid = cred->cc.cmcred_uid;
	      kcred->cc.gid = cred->cc.cmcred_gid;
	    }
	  memcpy (&m, message, sizeof (struct msghdr));
	  m.msg_control = buf;
	  m.msg_controllen -= b - a;
	  return __syscall_sendmsg (fd, &m, flags);
	}
    }
  return __syscall_sendmsg (fd, message, flags);
}

weak_alias (__libc_sendmsg, __sendmsg)
weak_alias (__libc_sendmsg, sendmsg)
