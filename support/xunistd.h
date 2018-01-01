/* POSIX-specific extra functions.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

/* These wrapper functions use POSIX types and therefore cannot be
   declared in <support/support.h>.  */

#ifndef SUPPORT_XUNISTD_H
#define SUPPORT_XUNISTD_H

#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>

__BEGIN_DECLS

struct stat64;

pid_t xfork (void);
pid_t xwaitpid (pid_t, int *status, int flags);
void xpipe (int[2]);
void xdup2 (int, int);
int xopen (const char *path, int flags, mode_t);
void xstat (const char *path, struct stat64 *);
void xfstat (int fd, struct stat64 *);
void xmkdir (const char *path, mode_t);
void xchroot (const char *path);
void xunlink (const char *path);
long xsysconf (int name);
long long xlseek (int fd, long long offset, int whence);
void xftruncate (int fd, long long length);

/* Read the link at PATH.  The caller should free the returned string
   with free.  */
char *xreadlink (const char *path);

/* Close the file descriptor.  Ignore EINTR errors, but terminate the
   process on other errors.  */
void xclose (int);

/* Write the buffer.  Retry on short writes.  */
void xwrite (int, const void *, size_t);

/* Invoke mmap with a zero file offset.  */
void *xmmap (void *addr, size_t length, int prot, int flags, int fd);
void xmprotect (void *addr, size_t length, int prot);
void xmunmap (void *addr, size_t length);

__END_DECLS

#endif /* SUPPORT_XUNISTD_H */
