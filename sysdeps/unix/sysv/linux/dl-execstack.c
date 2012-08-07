/* Stack executability handling for GNU dynamic linker.  Linux version.
   Copyright (C) 2003-2012 Free Software Foundation, Inc.
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

#include <ldsodefs.h>
#include <sys/mman.h>
#include <errno.h>
#include <libintl.h>
#include <stdbool.h>
#include <stackinfo.h>
#include <caller.h>
#include <sysdep.h>

#include <kernel-features.h>


extern int __stack_prot attribute_relro attribute_hidden;


int
internal_function
_dl_make_stack_executable (void **stack_endp)
{
  /* This gives us the highest/lowest page that needs to be changed.  */
  uintptr_t page = ((uintptr_t) *stack_endp
		    & -(intptr_t) GLRO(dl_pagesize));
  int result = 0;

  /* Challenge the caller.  */
  if (__builtin_expect (__check_caller (RETURN_ADDRESS (0),
					allow_ldso|allow_libpthread) != 0, 0)
      || __builtin_expect (*stack_endp != __libc_stack_end, 0))
    return EPERM;

  if (__builtin_expect (__mprotect ((void *) page, GLRO(dl_pagesize),
				    __stack_prot) == 0, 1))
    goto return_success;
  result = errno;
  goto out;

 return_success:
  /* Clear the address.  */
  *stack_endp = NULL;

  /* Remember that we changed the permission.  */
  GL(dl_stack_flags) |= PF_X;

 out:
#ifdef check_consistency
  check_consistency ();
#endif

  return result;
}
rtld_hidden_def (_dl_make_stack_executable)
