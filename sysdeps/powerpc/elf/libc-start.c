/* Copyright (C) 1998,2000,2001,2002 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <ldsodefs.h>
#include <bp-start.h>
#include <bp-sym.h>

extern void __libc_init_first (int argc, char **argv, char **envp);

extern int __cache_line_size;
weak_extern (__cache_line_size)

extern int __libc_multiple_libcs;
extern void *__libc_stack_end;

#ifndef SHARED
# include <tls.h>
extern void __pthread_initialize_minimal (void)
# if !(USE_TLS - 0) && !defined NONTLS_INIT_TP
     __attribute__ ((weak))
# endif
     ;
#endif

struct startup_info
{
  void *__unbounded sda_base;
  int (*main) (int, char **, char **, void *);
  int (*init) (int, char **, char **, void *);
  void (*fini) (void);
};

/* Scan the Aux Vector for the "Data Cache Block Size" entry.  If found
   verify that the static extern __cache_line_size is defined by checking
   for not NULL.  If it is defined then assign the cache block size
   value to __cache_line_size.  */
static inline void
__aux_init_cache (ElfW(auxv_t) *av)
{
  for (; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_DCACHEBSIZE:
        {
          int *cls = & __cache_line_size;
          if (cls != NULL)
            *cls = av->a_un.a_val;
        }
        break;
      }
}


int
/* GKM FIXME: GCC: this should get __BP_ prefix by virtue of the
   BPs in the arglist of startup_info.main and startup_info.init. */
BP_SYM (__libc_start_main) (int argc, char *__unbounded *__unbounded ubp_av,
		   char *__unbounded *__unbounded ubp_ev,
		   ElfW(auxv_t) *__unbounded auxvec, void (*rtld_fini) (void),
		   struct startup_info *__unbounded stinfo,
		   char *__unbounded *__unbounded stack_on_entry)
{
#if __BOUNDED_POINTERS__
  char **argv;
#else
# define argv ubp_av
#endif

#ifndef SHARED
  /* The next variable is only here to work around a bug in gcc <= 2.7.2.2.
     If the address would be taken inside the expression the optimizer
     would try to be too smart and throws it away.  Grrr.  */
  int *dummy_addr = &_dl_starting_up;

  __libc_multiple_libcs = dummy_addr && !_dl_starting_up;
#endif

  /* the PPC SVR4 ABI says that the top thing on the stack will
     be a NULL pointer, so if not we assume that we're being called
     as a statically-linked program by Linux...	 */
  if (*stack_on_entry != NULL)
    {
      char *__unbounded *__unbounded temp;
      /* ...in which case, we have argc as the top thing on the
	 stack, followed by argv (NULL-terminated), envp (likewise),
	 and the auxilary vector.  */
      /* 32/64-bit agnostic load from stack */
      argc = *(long int *__unbounded) stack_on_entry;
      ubp_av = stack_on_entry + 1;
      ubp_ev = ubp_av + argc + 1;
#ifdef HAVE_AUX_VECTOR
      temp = ubp_ev;
      while (*temp != NULL)
        ++temp;
      auxvec = (ElfW(auxv_t) *)++temp;

# ifndef SHARED
      _dl_aux_init (auxvec);
# endif
#endif
      rtld_fini = NULL;
    }

  INIT_ARGV_and_ENVIRON;

  /* Initialize the __cache_line_size variable from the aux vector.  */
  __aux_init_cache(auxvec);

  /* Store something that has some relationship to the end of the
     stack, for backtraces.  This variable should be thread-specific.
     Use +8 so it works for both 32- and 64-bit.  */
  __libc_stack_end = stack_on_entry + 8;

#ifndef SHARED
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
  if (rtld_fini != NULL)
    __cxa_atexit ((void (*) (void *)) rtld_fini, NULL, NULL);

  /* Call the initializer of the libc.  */
#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ninitialize libc\n\n");
#endif
  __libc_init_first (argc, argv, __environ);

  /* Register the destructor of the program, if any.  */
  if (stinfo->fini)
    __cxa_atexit ((void (*) (void *)) stinfo->fini, NULL, NULL);

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ninitialize program: %s\n\n", argv[0]);
#endif
  if (stinfo->init)
    stinfo->init (argc, argv, __environ, auxvec);

#ifdef SHARED
  if (__builtin_expect (GL(dl_debug_mask) & DL_DEBUG_IMPCALLS, 0))
    _dl_debug_printf ("\ntransferring control: %s\n\n", argv[0]);
#endif

  exit (stinfo->main (argc, argv, __environ, auxvec));
}
