/* Copyright (C) 1991, 1996, 1997, 2000-2002, 2003, 2004
   Free Software Foundation, Inc.
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

#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <confstr.h>
#include "../version.h"

/* If BUF is not NULL and LEN > 0, fill in at most LEN - 1 bytes
   of BUF with the value corresponding to NAME and zero-terminate BUF.
   Return the number of bytes required to hold NAME's entire value.  */
size_t
confstr (name, buf, len)
     int name;
     char *buf;
     size_t len;
{
  const char *string = "";
  size_t string_len = 1;

  switch (name)
    {
    case _CS_PATH:
      {
	static const char cs_path[] = CS_PATH;
	string = cs_path;
	string_len = sizeof (cs_path);
      }
      break;

    case _CS_V6_WIDTH_RESTRICTED_ENVS:
      /* We have to return a newline-separated list of named of
	 programming environements in which the widths of blksize_t,
	 cc_t, mode_t, nfds_t, pid_t, ptrdiff_t, size_t, speed_t,
	 ssize_t, suseconds_t, tcflag_t, useconds_t, wchar_t, and
	 wint_t types are no greater than the width of type long.

	 Currently this means all environment which the system allows.  */
      {
	char restenvs[4 * sizeof "POSIX_V6_LPBIG_OFFBIG"];

	string_len = 0;
#ifndef _POSIX_V6_ILP32_OFF32
        if (__sysconf (_SC_V6_ILP32_OFF32) > 0)
#endif
#if !defined _POSIX_V6_ILP32_OFF32 || _POSIX_V6_ILP32_OFF32 > 0
          {
            memcpy (restenvs + string_len, "POSIX_V6_ILP32_OFF32",
                    sizeof "POSIX_V6_ILP32_OFF32" - 1);
            string_len += sizeof "POSIX_V6_ILP32_OFF32" - 1;
          }
#endif
#ifndef _POSIX_V6_ILP32_OFFBIG
        if (__sysconf (_SC_V6_ILP32_OFFBIG) > 0)
#endif
#if !defined _POSIX_V6_ILP32_OFFBIG || _POSIX_V6_ILP32_OFFBIG > 0
          {
            if (string_len)
              restenvs[string_len++] = '\n';
            memcpy (restenvs + string_len, "POSIX_V6_ILP32_OFFBIG",
                    sizeof "POSIX_V6_ILP32_OFFBIG" - 1);
            string_len += sizeof "POSIX_V6_ILP32_OFFBIG" - 1;
          }
#endif
#ifndef _POSIX_V6_LP64_OFF64
        if (__sysconf (_SC_V6_LP64_OFF64) > 0)
#endif
#if !defined _POSIX_V6_LP64_OFF64 || _POSIX_V6_LP64_OFF64 > 0
          {
            if (string_len)
              restenvs[string_len++] = '\n';
            memcpy (restenvs + string_len, "POSIX_V6_LP64_OFF64",
                    sizeof "POSIX_V6_LP64_OFF64" - 1);
            string_len += sizeof "POSIX_V6_LP64_OFF64" - 1;
          }
#endif
#ifndef _POSIX_V6_LPBIG_OFFBIG
        if (__sysconf (_SC_V6_LPBIG_OFFBIG) > 0)
#endif
#if !defined _POSIX_V6_LPBIG_OFFBIG || _POSIX_V6_LPBIG_OFFBIG > 0
          {
            if (string_len)
              restenvs[string_len++] = '\n';
            memcpy (restenvs + string_len, "POSIX_V6_LPBIG_OFFBIG",
                    sizeof "POSIX_V6_LPBIG_OFFBIG" - 1);
            string_len += sizeof "POSIX_V6_LPBIG_OFFBIG" - 1;
          }
#endif
        restenvs[string_len++] = '\0';
	string = restenvs;
      }
      break;

    case _CS_XBS5_ILP32_OFF32_CFLAGS:
    case _CS_POSIX_V6_ILP32_OFF32_CFLAGS:
#ifdef __ILP32_OFF32_CFLAGS
# if _POSIX_V6_ILP32_OFF32 == -1
#  error "__ILP32_OFF32_CFLAGS should not be defined"
# elif !defined _POSIX_V6_ILP32_OFF32
      if (__sysconf (_SC_V6_ILP32_OFF32) < 0)
        break;
# endif
      string = __ILP32_OFF32_CFLAGS;
      string_len = sizeof (__ILP32_OFF32_CFLAGS);
#endif
      break;

    case _CS_XBS5_ILP32_OFFBIG_CFLAGS:
    case _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS:
#ifdef __ILP32_OFFBIG_CFLAGS
# if _POSIX_V6_ILP32_OFFBIG == -1
#  error "__ILP32_OFFBIG_CFLAGS should not be defined"
# elif !defined _POSIX_V6_ILP32_OFFBIG
      if (__sysconf (_SC_V6_ILP32_OFFBIG) < 0)
        break;
# endif
      string = __ILP32_OFFBIG_CFLAGS;
      string_len = sizeof (__ILP32_OFFBIG_CFLAGS);
#endif
      break;

    case _CS_XBS5_LP64_OFF64_CFLAGS:
    case _CS_POSIX_V6_LP64_OFF64_CFLAGS:
#ifdef __LP64_OFF64_CFLAGS
# if _POSIX_V6_LP64_OFF64 == -1
#  error "__LP64_OFF64_CFLAGS should not be defined"
# elif !defined _POSIX_V6_LP64_OFF64
      if (__sysconf (_SC_V6_LP64_OFF64) < 0)
        break;
# endif
      string = __LP64_OFF64_CFLAGS;
      string_len = sizeof (__LP64_OFF64_CFLAGS);
#endif
      break;

    case _CS_XBS5_ILP32_OFF32_LDFLAGS:
    case _CS_POSIX_V6_ILP32_OFF32_LDFLAGS:
#ifdef __ILP32_OFF32_LDFLAGS
# if _POSIX_V6_ILP32_OFF32 == -1
#  error "__ILP32_OFF32_LDFLAGS should not be defined"
# elif !defined _POSIX_V6_ILP32_OFF32
      if (__sysconf (_SC_V6_ILP32_OFF32) < 0)
        break;
# endif
      string = __ILP32_OFF32_LDFLAGS;
      string_len = sizeof (__ILP32_OFF32_LDFLAGS);
#endif
      break;

    case _CS_XBS5_ILP32_OFFBIG_LDFLAGS:
    case _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS:
#ifdef __ILP32_OFFBIG_LDFLAGS
# if _POSIX_V6_ILP32_OFFBIG == -1
#  error "__ILP32_OFFBIG_LDFLAGS should not be defined"
# elif !defined _POSIX_V6_ILP32_OFFBIG
      if (__sysconf (_SC_V6_ILP32_OFFBIG) < 0)
        break;
# endif
      string = __ILP32_OFFBIG_LDFLAGS;
      string_len = sizeof (__ILP32_OFFBIG_LDFLAGS);
#endif
      break;

    case _CS_XBS5_LP64_OFF64_LDFLAGS:
    case _CS_POSIX_V6_LP64_OFF64_LDFLAGS:
#ifdef __LP64_OFF64_LDFLAGS
# if _POSIX_V6_LP64_OFF64 == -1
#  error "__LP64_OFF64_LDFLAGS should not be defined"
# elif !defined _POSIX_V6_LP64_OFF64
      if (__sysconf (_SC_V6_LP64_OFF64) < 0)
        break;
# endif
      string = __LP64_OFF64_LDFLAGS;
      string_len = sizeof (__LP64_OFF64_LDFLAGS);
#endif
      break;

    case _CS_LFS_CFLAGS:
    case _CS_LFS_LINTFLAGS:
#if _POSIX_V6_ILP32_OFF32 == 1 && _POSIX_V6_ILP32_OFFBIG == 1
# define __LFS_CFLAGS "-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64"
      /* Signal that we want the new ABI.  */
      string = __LFS_CFLAGS;
      string_len = sizeof (__LFS_CFLAGS);
#endif
      break;

    case _CS_LFS_LDFLAGS:
    case _CS_LFS_LIBS:
      /* No special libraries or linker flags needed.  */
      break;

    case _CS_LFS64_CFLAGS:
    case _CS_LFS64_LINTFLAGS:
#define __LFS64_CFLAGS "-D_LARGEFILE64_SOURCE"
      string = __LFS64_CFLAGS;
      string_len = sizeof (__LFS64_CFLAGS);
      break;

    case _CS_LFS64_LDFLAGS:
    case _CS_LFS64_LIBS:
      /* No special libraries or linker flags needed.  */
      break;

    case _CS_XBS5_ILP32_OFF32_LIBS:
    case _CS_XBS5_ILP32_OFF32_LINTFLAGS:
    case _CS_XBS5_ILP32_OFFBIG_LIBS:
    case _CS_XBS5_ILP32_OFFBIG_LINTFLAGS:
    case _CS_XBS5_LP64_OFF64_LIBS:
    case _CS_XBS5_LP64_OFF64_LINTFLAGS:
    case _CS_XBS5_LPBIG_OFFBIG_CFLAGS:
    case _CS_XBS5_LPBIG_OFFBIG_LDFLAGS:
    case _CS_XBS5_LPBIG_OFFBIG_LIBS:
    case _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS:

    case _CS_POSIX_V6_ILP32_OFF32_LIBS:
    case _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS:
    case _CS_POSIX_V6_ILP32_OFFBIG_LIBS:
    case _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS:
    case _CS_POSIX_V6_LP64_OFF64_LIBS:
    case _CS_POSIX_V6_LP64_OFF64_LINTFLAGS:
    case _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS:
    case _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS:
    case _CS_POSIX_V6_LPBIG_OFFBIG_LIBS:
    case _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS:
      /* GNU libc does not require special actions to use LFS functions.  */
      break;

    case _CS_GNU_LIBC_VERSION:
      string = "glibc " VERSION;
      string_len = sizeof ("glibc " VERSION);
      break;

    case _CS_GNU_LIBPTHREAD_VERSION:
#ifdef LIBPTHREAD_VERSION
      string = LIBPTHREAD_VERSION;
      string_len = sizeof LIBPTHREAD_VERSION;
      break;
#else
      /* No thread library.  */
      __set_errno (EINVAL);
      return 0;
#endif

    default:
      __set_errno (EINVAL);
      return 0;
    }

  if (len > 0 && buf != NULL)
    {
      if (string_len <= len)
	memcpy (buf, string, string_len);
      else
	{
	  memcpy (buf, string, len - 1);
	  buf[len - 1] = '\0';
	}
    }
  return string_len;
}
libc_hidden_def (confstr)
