/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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

struct startup_info
{
  void *__unbounded sda_base;
  int (*main) (int, char **, char **, void *);
  int (*init) (int, char **, char **, void *);
  void (*fini) (void);
};

int
/* GKM FIXME: GCC: this should get __BP_ prefix by virtue of the
   BPs in the arglist of startup_info.main and startup_info.init. */
BP_SYM (__libc_start_main) (int argc, char *__unbounded *__unbounded ubp_av,
		   char *__unbounded *__unbounded ubp_ev,
		   void *__unbounded auxvec, void (*rtld_fini) (void),
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
      /* ...in which case, we have argc as the top thing on the
	 stack, followed by argv (NULL-terminated), envp (likewise),
	 and the auxilary vector.  */
      argc = *(int *__unbounded) stack_on_entry;
      ubp_av = stack_on_entry + 1;
      ubp_ev = ubp_av + argc + 1;
      auxvec = ubp_ev;
      while (*(char *__unbounded *__unbounded) auxvec != NULL)
	++auxvec;
      ++auxvec;
      _dl_aux_init ((ElfW(auxv_t) *) auxvec);
      rtld_fini = NULL;
    }

  INIT_ARGV_and_ENVIRON;

  /* Store something that has some relationship to the end of the
     stack, for backtraces.  This variable should be thread-specific.  */
  __libc_stack_end = stack_on_entry + 4;

  /* Register the destructor of the dynamic linker if there is any.  */
  if (rtld_fini != NULL)
    atexit (rtld_fini);

  /* Call the initializer of the libc.  */
#ifdef SHARED
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize libc\n\n", NULL);
#endif
  __libc_init_first (argc, argv, __environ);

  /* Register the destructor of the program, if any.  */
  if (stinfo->fini)
    atexit (stinfo->fini);

  /* Call the initializer of the program, if any.  */
#ifdef SHARED
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ninitialize program: ", argv[0], "\n\n", NULL);
#endif
  if (stinfo->init)
    stinfo->init (argc, argv, __environ, auxvec);

#ifdef SHARED
  if (_dl_debug_impcalls)
    _dl_debug_message (1, "\ntransferring control: ", argv[0], "\n\n", NULL);
#endif

  exit (stinfo->main (argc, argv, __environ, auxvec));
}
