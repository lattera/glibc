/* Catastrophic failure reports.  Generic POSIX.1 version.
   Copyright (C) 1993-2018 Free Software Foundation, Inc.
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
#include <sys/uio.h>
#include <not-cancel.h>

#ifdef FATAL_PREPARE_INCLUDE
#include FATAL_PREPARE_INCLUDE
#endif

#ifndef WRITEV_FOR_FATAL
# define WRITEV_FOR_FATAL	writev_for_fatal
static bool
writev_for_fatal (int fd, const struct iovec *iov, size_t niov, size_t total)
{
  return TEMP_FAILURE_RETRY (__writev (fd, iov, niov)) == total;
}
#endif

#ifndef BEFORE_ABORT
# define BEFORE_ABORT		before_abort
static void
before_abort (int do_abort __attribute__ ((unused)),
              bool written __attribute__ ((unused)),
              int fd __attribute__ ((unused)))
{
}
#endif

struct str_list
{
  const char *str;
  size_t len;
  struct str_list *next;
};

/* Abort with an error message.  */
void
__libc_message (enum __libc_message_action action, const char *fmt, ...)
{
  va_list ap;
  int fd = -1;

  va_start (ap, fmt);

#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  /* Don't call __libc_secure_getenv if we aren't doing backtrace, which
     may access the corrupted stack.  */
  if ((action & do_backtrace))
    {
      /* Open a descriptor for /dev/tty unless the user explicitly
	 requests errors on standard error.  */
      const char *on_2 = __libc_secure_getenv ("LIBC_FATAL_STDERR_");
      if (on_2 == NULL || *on_2 == '\0')
	fd = __open_nocancel (_PATH_TTY, O_RDWR | O_NOCTTY | O_NDELAY);
    }

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
	  iov[cnt].iov_base = (char *) list->str;
	  iov[cnt].iov_len = list->len;
	  total += list->len;
	  list = list->next;
	}

      written = WRITEV_FOR_FATAL (fd, iov, nlist, total);

      if ((action & do_abort))
	{
	  total = ((total + 1 + GLRO(dl_pagesize) - 1)
		   & ~(GLRO(dl_pagesize) - 1));
	  struct abort_msg_s *buf = __mmap (NULL, total,
					    PROT_READ | PROT_WRITE,
					    MAP_ANON | MAP_PRIVATE, -1, 0);
	  if (__glibc_likely (buf != MAP_FAILED))
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

  if ((action & do_abort))
    {
      if ((action & do_backtrace))
	BEFORE_ABORT (do_abort, written, fd);

      /* Kill the application.  */
      abort ();
    }
}


void
__libc_fatal (const char *message)
{
  /* The loop is added only to keep gcc happy.  */
  while (1)
    __libc_message (do_abort | do_backtrace, "%s", message);
}
libc_hidden_def (__libc_fatal)
