/* Copyright (C) 1998, 2000, 2001, 2002, 2003, 2004, 2005, 2007
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <unistd.h>
#include <ldsodefs.h>
#include <sysdep.h>
#include <bp-start.h>
#include <bp-sym.h>


int __cache_line_size attribute_hidden;
/* The main work is done in the generic function.  */
#define LIBC_START_MAIN generic_start_main
#define LIBC_START_DISABLE_INLINE
#define LIBC_START_MAIN_AUXVEC_ARG
#define MAIN_AUXVEC_ARG
#define INIT_MAIN_ARGS
#include <csu/libc-start.c>

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
			      ElfW (auxv_t) * __unbounded auxvec,
			      void (*rtld_fini) (void),
			      struct startup_info *__unbounded stinfo,
			      char *__unbounded *__unbounded stack_on_entry)
{
#if __BOUNDED_POINTERS__
  char **argv;
#else
# define argv ubp_av
#endif

  /* the PPC SVR4 ABI says that the top thing on the stack will
     be a NULL pointer, so if not we assume that we're being called
     as a statically-linked program by Linux...  */
  if (*stack_on_entry != NULL)
    {
      char *__unbounded * __unbounded temp;
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
      auxvec = (ElfW (auxv_t) *)++ temp;
#endif
      rtld_fini = NULL;
    }

  /* Initialize the __cache_line_size variable from the aux vector.  */
  for (ElfW (auxv_t) * av = auxvec; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_DCACHEBSIZE:
	__cache_line_size = av->a_un.a_val;
	break;
      }

  return generic_start_main (stinfo->main, argc, ubp_av, auxvec,
			     stinfo->init, stinfo->fini, rtld_fini,
			     stack_on_entry);
}
