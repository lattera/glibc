/* Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <bp-start.h>
#include <bp-sym.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int _dl_starting_up;
weak_extern (_dl_starting_up)
extern int __libc_multiple_libcs;
extern void *__libc_stack_end;

#ifndef SHARED
extern void __pthread_initialize_minimal (void) __attribute__ ((weak));
#endif


extern int BP_SYM (__libc_start_main) (int (*main) (int, char **, char **),
				       int argc,
				       char *__unbounded *__unbounded ubp_av,
				       void (*init) (void),
				       void (*fini) (void),
				       void (*rtld_fini) (void),
				       void *__unbounded stack_end)
     __attribute__ ((noreturn));

int
/* GKM FIXME: GCC: this should get __BP_ prefix by virtue of the
   BPs in the arglist of startup_info.main and startup_info.init. */
BP_SYM (__libc_start_main) (int (*main) (int, char **, char **),
		   int argc, char *__unbounded *__unbounded ubp_av,
		   void (*init) (void), void (*fini) (void),
		   void (*rtld_fini) (void), void *__unbounded stack_end)
{
  char *__unbounded *__unbounded ubp_ev = &ubp_av[argc + 1];
#if __BOUNDED_POINTERS__
  char **argv;
#else
# define argv ubp_av
#endif

#ifndef SHARED
# ifdef HAVE_AUX_VECTOR
  void *__unbounded *__unbounded auxvec;
# endif

  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.
     If the address would be taken inside the expression the optimizer
     would try to be too smart and throws it away.  Grrr.  */
  int *dummy_addr = &_dl_starting_up;

  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;
#endif

  INIT_ARGV_and_ENVIRON;

  /* Store the lowest stack address.  */
  __libc_stack_end = stack_end;

#ifndef SHARED
  /* Initialize the thread library at least a bit since the libgcc
     functions are using thread functions if these are available and
     we need to setup errno.  */
  if (__pthread_initialize_minimal)
    __pthread_initialize_minimal ();

  /* Some security at this point.  Prevent starting a SUID binary where
     the standard file descriptors are not opened.  We have to do this
     only for statically linked applications since otherwise the dynamic
     loader did the work already.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    __libc_check_standard_fds ();

# ifdef HAVE_AUX_VECTOR
  for (auxvec = (void *__unbounded *__unbounded) ubp_ev;
       *auxvec; auxvec++);
  ++auxvec;
  _dl_aux_init ((ElfW(auxv_t) *) auxvec);
# endif
#endif

  /* Register the destructor of the dynamic linker if there is any.  */
  if (__builtin_expect (rtld_fini != NULL, 1))
    atexit (rtld_fini);

  /* Call the initializer of the libc.  This is only needed here if we
     are compiling for the static library in which case we haven't
     run the constructors in `_dl_start_user'.  */
#ifndef SHARED
  __libc_init_first (argc, argv, __environ);
#endif

  /* Register the destructor of the program, if any.  */
  if (fini)
    atexit (fini);

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (__builtin_expect (_dl_debug_impcalls, 0))
    _dl_debug_message (1, "\ninitialize program: ", argv[0], "\n\n", NULL);
#endif
  if (init)
    (*init) ();

#ifdef SHARED
  if (__builtin_expect (_dl_debug_impcalls, 0))
    _dl_debug_message (1, "\ntransferring control: ", argv[0], "\n\n", NULL);
#endif

  exit ((*main) (argc, argv, __environ));
}
