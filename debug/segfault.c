/* Catch segmentation faults and print backtrace.
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <alloca.h>
#include <ctype.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio-common/_itoa.h>
#include <ldsodefs.h>

#include <bp-checks.h>

/* This file defines macros to access the content of the sigcontext element
   passed up by the signal handler.  */
#include <sigcontextinfo.h>

/* Get code to possibly dump the content of all registers.  */
#include <register-dump.h>

/* We'll use tis a lot.  */
#define WRITE_STRING(s) write (fd, s, strlen (s))

/* Name of the output file.  */
static const char *fname;


/* We better should not use `strerror' since it can call far too many
   other functions which might fail.  Do it here ourselves.  */
static void
write_strsignal (int fd, int signal)
{
  if (signal < 0 || signal >= _NSIG || _sys_siglist[signal] == NULL)
    {
      char buf[30];
      char *ptr = _itoa_word (signal, &buf[sizeof (buf)], 10, 0);
      WRITE_STRING ("signal ");
      write (fd, buf, &buf[sizeof (buf)] - ptr);
    }
  else
    WRITE_STRING (_sys_siglist[signal]);
}


/* This function is called when a segmentation fault is caught.  The system
   is in an instable state now.  This means especially that malloc() might
   not work anymore.  */
static void
catch_segfault (int signal, SIGCONTEXT ctx)
{
  int fd, cnt, i;
  void **arr;
  struct sigaction sa;
  uintptr_t pc;

  /* This is the name of the file we are writing to.  If none is given
     or we cannot write to this file write to stderr.  */
  fd = 2;
  if (fname != NULL)
    {
      fd = open (fname, O_TRUNC | O_WRONLY | O_CREAT, 0666);
      if (fd == -1)
	fd = 2;
    }

  WRITE_STRING ("*** ");
  write_strsignal (fd, signal);
  WRITE_STRING ("\n");

#ifdef REGISTER_DUMP
  REGISTER_DUMP;
#endif

  WRITE_STRING ("\nBacktrace:\n");

  /* Get the backtrace.  */
  arr = alloca (256 * sizeof (void *));
  cnt = backtrace (arr, 256);

  /* Now try to locate the PC from signal context in the backtrace.
     Normally it will be found at arr[2], but it might appear later
     if there were some signal handler wrappers.  Allow a few bytes
     difference to cope with as many arches as possible.  */
  pc = (uintptr_t) GET_PC (ctx);
  for (i = 0; i < cnt; ++i)
    if ((uintptr_t) arr[i] >= pc - 16 && (uintptr_t) arr[i] <= pc + 16)
      break;

  /* If we haven't found it, better dump full backtrace even including
     the signal handler frames instead of not dumping anything.  */
  if (i == cnt)
    i = 0;

  /* Now generate nicely formatted output.  */
  __backtrace_symbols_fd (arr + i, cnt - i, fd);

#ifdef HAVE_PROC_SELF
  /* Now the link map.  */
  int mapfd = open ("/proc/self/maps", O_RDONLY);
  if (mapfd != -1)
    {
      write (fd, "\nMemory map:\n\n", 14);

      char buf[256];
      ssize_t n;

      while ((n = TEMP_FAILURE_RETRY (read (mapfd, buf, sizeof (buf)))) > 0)
	TEMP_FAILURE_RETRY (write (fd, buf, n));

      close (mapfd);
    }
#endif

  /* Pass on the signal (so that a core file is produced).  */
  sa.sa_handler = SIG_DFL;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction (signal, &sa, NULL);
  raise (signal);
}


static void
__attribute__ ((constructor))
install_handler (void)
{
  struct sigaction sa;
  const char *sigs = getenv ("SEGFAULT_SIGNALS");
  const char *name;

  sa.sa_handler = (void *) catch_segfault;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  /* Maybe we are expected to use an alternative stack.  */
  if (getenv ("SEGFAULT_USE_ALTSTACK") != 0)
    {
      void *stack_mem = malloc (2 * SIGSTKSZ);
      struct sigaltstack ss;

      if (stack_mem != NULL)
	{
	  ss.ss_sp = stack_mem;
	  ss.ss_flags = 0;
	  ss.ss_size = 2 * SIGSTKSZ;

	  if (sigaltstack (&ss, NULL) == 0)
	    sa.sa_flags |= SA_ONSTACK;
	}
    }

  if (sigs == NULL)
    sigaction (SIGSEGV, &sa, NULL);
  else if (sigs[0] == '\0')
    /* Do not do anything.  */
    return;
  else
    {
      const char *where;
      int all = __strcasecmp (sigs, "all") == 0;

#define INSTALL_FOR_SIG(sig, name) \
      where = __strcasestr (sigs, name);				      \
      if (all || (where != NULL						      \
		  && (where == sigs || !isalnum (where[-1]))		      \
		  && !isalnum (where[sizeof (name) - 1])))		      \
	sigaction (sig, &sa, NULL);

      INSTALL_FOR_SIG (SIGSEGV, "segv");
      INSTALL_FOR_SIG (SIGILL, "ill");
#ifdef SIGBUS
      INSTALL_FOR_SIG (SIGBUS, "bus");
#endif
#ifdef SIGSTKFLT
      INSTALL_FOR_SIG (SIGSTKFLT, "stkflt");
#endif
      INSTALL_FOR_SIG (SIGABRT, "abrt");
      INSTALL_FOR_SIG (SIGFPE, "fpe");
    }

  /* Preserve the output file name if there is any given.  */
  name = getenv ("SEGFAULT_OUTPUT_NAME");
  if (name != NULL && name[0] != '\0')
    {
      int ret = access (name, R_OK | W_OK);

      if (ret == 0 || (ret == -1 && errno == ENOENT))
	fname = __strdup (name);
    }
}
