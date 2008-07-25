/* Copyright (C) 1999, 2001, 2002, 2007, 2008 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <kernel-features.h>

/* Return a socket of any type.  The socket can be used in subsequent
   ioctl calls to talk to the kernel.  */
int internal_function
__opensock (void)
{
  static int last_family;	/* Available socket family we will use.  */
  static int last_type;
  static const struct
  {
    int family;
    const char procname[15];
  } afs[] =
    {
      /* The 2.2 kernels cannot handle ioctl(SIOCGIFCONF) on AF_UNIX sockets.
	 Give the kernel a chance to user inet sockets on old kernels.  */
#if __LINUX_KERNEL_VERSION < 132096
      { AF_INET, "" },
      { AF_UNIX, "net/unix" },
#else
      { AF_UNIX, "net/unix" },
      { AF_INET, "" },
#endif
      { AF_INET6, "net/if_inet6" },
      { AF_AX25, "net/ax25" },
      { AF_NETROM, "net/nr" },
      { AF_ROSE, "net/rose" },
      { AF_IPX, "net/ipx" },
      { AF_APPLETALK, "net/appletalk" },
      { AF_ECONET, "sys/net/econet" },
      { AF_ASH, "sys/net/ash" },
      { AF_X25, "net/x25" },
#ifdef NEED_AF_IUCV
      { AF_IUCV, "net/iucv" }
#endif
    };
#define nafs (sizeof (afs) / sizeof (afs[0]))
  char fname[sizeof "/proc/" + 14];
  int result;
  int has_proc;
  size_t cnt;

  /* We already know which family to use from the last call.  Use it
     again.  */
  if (last_family != 0)
    {
      assert (last_type != 0);

#ifdef SOCK_CLOEXEC
# ifndef __ASSUME_SOCK_CLOEXEC
      if (__have_sock_cloexec >= 0)
# endif
	{
	  result = __socket (last_family, last_type | SOCK_CLOEXEC, 0);
# ifndef __ASSUME_SOCK_CLOEXEC
	  if (__have_sock_cloexec == 0)
	    __have_sock_cloexec = result != -1 || errno != EINVAL ? 1 : -1;
# endif
	}
#endif
#ifndef __ASSUME_SOCK_CLOEXEC
# ifdef SOCK_CLOEXEC
      if (__have_sock_cloexec < 0)
# endif
	result = __socket (last_family, last_type, 0);
#endif
      if (result != -1 || errno != EAFNOSUPPORT)
	/* Maybe the socket type isn't supported anymore (module is
	   unloaded).  In this case again try to find the type.  */
	return result;

      /* Reset the values.  They seem not valid anymore.  */
      last_family = 0;
      last_type = 0;
    }

  /* Check whether the /proc filesystem is available.  */
  has_proc = __access ("/proc/net", R_OK) != -1;
  strcpy (fname, "/proc/");

  /* Iterate over the interface families and find one which is
     available.  */
  for (cnt = 0; cnt < nafs; ++cnt)
    {
      int type = SOCK_DGRAM;

      if (has_proc && afs[cnt].procname[0] != '\0')
	{
	  strcpy (fname + 6, afs[cnt].procname);
	  if (__access (fname, R_OK) == -1)
	    /* The /proc entry is not available.  I.e., we cannot
	       create a socket of this type (without loading the
	       module).  Don't look for it since this might trigger
	       loading the module.  */
	    continue;
	}

      if (afs[cnt].family == AF_NETROM || afs[cnt].family == AF_X25)
	type = SOCK_SEQPACKET;

#ifdef SOCK_CLOEXEC
# ifndef __ASSUME_SOCK_CLOEXEC
      if (__have_sock_cloexec >= 0)
# endif
	{
	  result = __socket (afs[cnt].family, type | SOCK_CLOEXEC, 0);
# ifndef __ASSUME_SOCK_CLOEXEC
	  if (__have_sock_cloexec == 0)
	    __have_sock_cloexec = result != -1 || errno != EINVAL ? 1 : -1;
# endif
	}
#endif
#ifndef __ASSUME_SOCK_CLOEXEC
# ifdef SOCK_CLOEXEC
      if (__have_sock_cloexec < 0)
# endif
	result = __socket (afs[cnt].family, type, 0);
#endif
      if (result != -1)
	{
	  /* Found an available family.  */
	  last_type = type;
	  last_family = afs[cnt].family;
	  return result;
	}
    }

  /* None of the protocol families is available.  It is unclear what kind
     of error is returned.  ENOENT seems like a reasonable choice.  */
  __set_errno (ENOENT);
  return -1;
}
