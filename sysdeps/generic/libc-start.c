/* Copyright (C) 1998, 1999 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <elf/ldsodefs.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int _dl_starting_up;
weak_extern (_dl_starting_up)
extern int __libc_multiple_libcs;
extern void *__libc_stack_end;

/* Prototype for local function.  */
static void check_standard_fds (void);

int
__libc_start_main (int (*main) (int, char **, char **), int argc,
		   char **argv, void (*init) (void), void (*fini) (void),
		   void (*rtld_fini) (void), void *stack_end)
{
#ifndef PIC
  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.
     If the address would be taken inside the expression the optimizer
     would try to be too smart and throws it away.  Grrr.  */
  int *dummy_addr = &_dl_starting_up;

  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;
#endif

  /* Store the lowest stack address.  */
  __libc_stack_end = stack_end;

  /* Set the global _environ variable correctly.  */
  __environ = &argv[argc + 1];

  /* Some security at this point.  Prevent starting a SUID binary where
     the standard file descriptors are not opened.  */
  if (__libc_enable_secure)
    check_standard_fds ();

  /* Register the destructor of the dynamic linker if there is any.  */
  if (rtld_fini != NULL)
    atexit (rtld_fini);

  /* Call the initializer of the libc.  */
#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize libc\n\n", NULL);
#endif
  __libc_init_first (argc, argv, __environ);

  /* Register the destructor of the program, if any.  */
  if (fini)
    atexit (fini);

  /* Call the initializer of the program, if any.  */
#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize program: ", argv[0], "\n\n", NULL);
#endif
  if (init)
    (*init) ();

#ifdef PIC
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ntransferring control: ", argv[0], "\n\n", NULL);
#endif

  exit ((*main) (argc, argv, __environ));
}


/* Should other OSes (e.g., Hurd) have different versions which can
   be written in a better way?  */
static void
check_one_fd (int fd, int mode)
{
  if (__fcntl (fd, F_GETFD) == -1 && errno == EBADF)
    {
      /* Something is wrong with this descriptor, it's probably not
	 opened.  Open /dev/null so that the SUID program we are
	 about to start does not accidently use this descriptor.  */
      int nullfd = __open (_PATH_DEVNULL, mode);
      if (nullfd == -1)
	/* We cannot even given an error message here since it would
	   run into the same problems.  */
	abort ();
    }
}


static void
check_standard_fds (void)
{
/* Check all three standard file descriptors.  */
  check_one_fd (STDIN_FILENO, O_RDONLY);
  check_one_fd (STDOUT_FILENO, O_RDWR);
  check_one_fd (STDERR_FILENO, O_RDWR);
}
