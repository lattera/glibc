/* Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifdef SHARED
# include <csu/libc-start.c>
# else
/* The main work is done in the generic function.  */
# define LIBC_START_DISABLE_INLINE
# define LIBC_START_MAIN generic_start_main
# include <csu/libc-start.c>
# include <cpu-features.h>
# include <cpu-features.c>

extern struct cpu_features _dl_x86_cpu_features;

int
__libc_start_main (int (*main) (int, char **, char ** MAIN_AUXVEC_DECL),
		   int argc, char **argv,
		   __typeof (main) init,
		   void (*fini) (void),
		   void (*rtld_fini) (void), void *stack_end)
{
  init_cpu_features (&_dl_x86_cpu_features);
  return generic_start_main (main, argc, argv, init, fini, rtld_fini,
			     stack_end);
}
#endif
