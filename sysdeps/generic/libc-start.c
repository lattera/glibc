/* Copyright (C) 1998,1999,2000,2001,2002,2003 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <bp-start.h>
#include <bp-sym.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int __libc_multiple_libcs;
extern void *__libc_stack_end;

#include <tls.h>
#ifndef SHARED
# include <dl-osinfo.h>
extern void __pthread_initialize_minimal (void)
# if !(USE_TLS - 0) && !defined NONTLS_INIT_TP
     __attribute__ ((weak))
# endif
     ;
#endif

#ifdef HAVE_PTR_NTHREADS
/* We need atomic operations.  */
# include <atomic.h>
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

  /* Result of the 'main' function.  */
  int result;

  __libc_multiple_libcs = &_dl_starting_up && !_dl_starting_up;

  INIT_ARGV_and_ENVIRON;

  /* Store the lowest stack address.  */
  __libc_stack_end = stack_end;

#ifndef SHARED
# ifdef HAVE_AUX_VECTOR
  void *__unbounded *__unbounded auxvec;
  /* First process the auxiliary vector since we need to find the
     program header to locate an eventually present PT_TLS entry.  */
  for (auxvec = (void *__unbounded *__unbounded) ubp_ev;
       *auxvec != NULL; ++auxvec);
  ++auxvec;
  _dl_aux_init ((ElfW(auxv_t) *) auxvec);
# endif
# ifdef DL_SYSDEP_OSCHECK
  if (!__libc_multiple_libcs)
    {
      /* This needs to run to initiliaze _dl_osversion before TLS
	 setup might check it.  */
      DL_SYSDEP_OSCHECK (__libc_fatal);
    }
# endif

  /* Initialize the thread library at least a bit since the libgcc
     functions are using thread functions if these are available and
     we need to setup errno.  If there is no thread library and we
     handle TLS the function is defined in the libc to initialized the
     TLS handling.  */
# if !(USE_TLS - 0) && !defined NONTLS_INIT_TP
  if (__pthread_initialize_minimal)
# endif
    __pthread_initialize_minimal ();

  /* Some security at this point.  Prevent starting a SUID binary where
     the standard file descriptors are not opened.  We have to do this
     only for statically linked applications since otherwise the dynamic
     loader did the work already.  */
  if (__builtin_expect (__libc_enable_secure, 0))
    __libc_check_standard_fds ();
#endif

  /* Register the destructor of the dynamic linker if there is any.  */
  if (__builtin_expect (rtld_fini != NULL, 1))
    __cxa_atexit ((void (*) (void *)) rtld_fini, NULL, NULL);

  /* Call the initializer of the libc.  This is only needed here if we
     are compiling for the static library in which case we haven't
     run the constructors in `_dl_start_user'.  */
#ifndef SHARED
  __libc_init_first (argc, argv, __environ);
#endif

  /* Register the destructor of the program, if any.  */
  if (fini)
    __cxa_atexit ((void (*) (void *)) fini, NULL, NULL);

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ninitialize program: %s\n\n", argv[0]);
#endif
  if (init)
    (*init) ();

#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ntransferring control: %s\n\n", argv[0]);
#endif

#ifdef HAVE_CANCELBUF
  if (setjmp (THREAD_SELF->cancelbuf) == 0)
#endif
    {
      /* XXX This is where the try/finally handling must be used.  */

      result = main (argc, argv, __environ);
    }
#ifdef HAVE_CANCELBUF
  else
    {
# ifdef HAVE_PTR_NTHREADS
      /* One less thread.  Decrement the counter.  If it is zero we
	 terminate the entire process.  */
      result = 0;
      int *const ptr;
#  ifdef SHARED
      ptr = __libc_pthread_functions.ptr_nthreads;
#  else
      extern int __nptl_nthreads __attribute ((weak));
      ptr = &__nptl_nthreads;
#  endif

      if (! atomic_decrement_and_test (ptr))
# endif
	/* Not much left to do but to exit the thread, not the process.  */
	__exit_thread (0);
    }
#endif

  exit (result);
}
