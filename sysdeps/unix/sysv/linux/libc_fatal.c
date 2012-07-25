/* Copyright (C) 1993-2012 Free Software Foundation, Inc.
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

#include <atomic.h>
#include <errno.h>
#include <fcntl.h>
#include <ldsodefs.h>
#include <paths.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdep.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syslog.h>
#include <execinfo.h>

/* Abort with an error message.  */
#include <not-cancel.h>

#ifdef FATAL_PREPARE_INCLUDE
#include FATAL_PREPARE_INCLUDE
#endif

struct str_list
{
  const char *str;
  size_t len;
  struct str_list *next;
};


/* Abort with an error message.  */
void
__libc_message (int do_abort, const char *fmt, ...)
{
  va_list ap;
  va_list ap_copy;
  int fd = -1;

  va_start (ap, fmt);
  va_copy (ap_copy, ap);

#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  /* Open a descriptor for /dev/tty unless the user explicitly
     requests errors on standard error.  */
  const char *on_2 = __libc_secure_getenv ("LIBC_FATAL_STDERR_");
  if (on_2 == NULL || *on_2 == '\0')
    fd = open_not_cancel_2 (_PATH_TTY, O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd == -1)
    fd = STDERR_FILENO;

  struct str_list *list = NULL;
  int nlist = 0;

  const char *cp = fmt;
  while (*cp != '\0')
    {
      /* Find the next "%s" or the end of the string.  */
      const char *next = cp;
      while (next[0] != '%' || next[1] != 's')
	{
	  next = __strchrnul (next + 1, '%');

	  if (next[0] == '\0')
	    break;
	}

      /* Determine what to print.  */
      const char *str;
      size_t len;
      if (cp[0] == '%' && cp[1] == 's')
	{
	  str = va_arg (ap, const char *);
	  len = strlen (str);
	  cp += 2;
	}
      else
	{
	  str = cp;
	  len = next - cp;
	  cp = next;
	}

      struct str_list *newp = alloca (sizeof (struct str_list));
      newp->str = str;
      newp->len = len;
      newp->next = list;
      list = newp;
      ++nlist;
    }

  bool written = false;
  if (nlist > 0)
    {
      struct iovec *iov = alloca (nlist * sizeof (struct iovec));
      ssize_t total = 0;

      for (int cnt = nlist - 1; cnt >= 0; --cnt)
	{
	  iov[cnt].iov_base = (void *) list->str;
	  iov[cnt].iov_len = list->len;
	  total += list->len;
	  list = list->next;
	}

      INTERNAL_SYSCALL_DECL (err);
      ssize_t cnt;
      do
	cnt = INTERNAL_SYSCALL (writev, err, 3, fd, iov, nlist);
      while (INTERNAL_SYSCALL_ERROR_P (cnt, err)
	     && INTERNAL_SYSCALL_ERRNO (cnt, err) == EINTR);

      if (cnt == total)
	written = true;

      if (do_abort)
	{
	  total = ((total + 1 + GLRO(dl_pagesize) - 1)
		   & ~(GLRO(dl_pagesize) - 1));
	  struct abort_msg_s *buf = __mmap (NULL, total,
					    PROT_READ | PROT_WRITE,
					    MAP_ANON | MAP_PRIVATE, -1, 0);
	  if (__builtin_expect (buf != MAP_FAILED, 1))
	    {
	      buf->size = total;
	      char *wp = buf->msg;
	      for (int cnt = 0; cnt < nlist; ++cnt)
		wp = mempcpy (wp, iov[cnt].iov_base, iov[cnt].iov_len);
	      *wp = '\0';

	      /* We have to free the old buffer since the application might
		 catch the SIGABRT signal.  */
	      struct abort_msg_s *old = atomic_exchange_acq (&__abort_msg,
							     buf);
	      if (old != NULL)
		__munmap (old, old->size);
	    }
	}
    }

  va_end (ap);

  /* If we  had no success writing the message, use syslog.  */
  if (! written)
    vsyslog (LOG_ERR, fmt, ap_copy);

  va_end (ap_copy);

  if (do_abort)
    {
      if (do_abort > 1 && written)
	{
	  void *addrs[64];
#define naddrs (sizeof (addrs) / sizeof (addrs[0]))
	  int n = __backtrace (addrs, naddrs);
	  if (n > 2)
	    {
#define strnsize(str) str, strlen (str)
#define writestr(str) write_not_cancel (fd, str)
	      writestr (strnsize ("======= Backtrace: =========\n"));
	      __backtrace_symbols_fd (addrs + 1, n - 1, fd);

	      writestr (strnsize ("======= Memory map: ========\n"));
	      int fd2 = open_not_cancel_2 ("/proc/self/maps", O_RDONLY);
	      char buf[1024];
	      ssize_t n2;
	      while ((n2 = read_not_cancel (fd2, buf, sizeof (buf))) > 0)
		if (write_not_cancel (fd, buf, n2) != n2)
		  break;
	      close_not_cancel_no_status (fd2);
	    }
	}

      /* Terminate the process.  */
      abort ();
    }
}


void
__libc_fatal (message)
     const char *message;
{
  /* The loop is added only to keep gcc happy.  */
  while (1)
    __libc_message (1, "%s", message);
}
libc_hidden_def (__libc_fatal)
