/* Stack executability handling for GNU dynamic linker.  Linux version.
   Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <ldsodefs.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>
#include <stackinfo.h>

extern void *__libc_stack_end;

int
internal_function
_dl_make_stack_executable (void)
{
  if (__libc_stack_end == 0)
    /* XXX for a DT_NEEDED library that requires the change,
       this is not initialized yet!
    */
    return ENOSYS;

#if _STACK_GROWS_DOWN
  /* This gives us the highest page that needs to be changed.  */
  uintptr_t page = (uintptr_t) __libc_stack_end & -(intptr_t) GL(dl_pagesize);

  /* There is always a hole in the address space below the bottom of the
     stack.  So when we make an mprotect call that starts below the bottom
     of the stack, it will include the hole and fail with ENOMEM.

     We start with a random guess at how deep the stack might have gotten
     so as to have extended the GROWSDOWN mapping to lower pages.  */

  size_t size = GL(dl_pagesize) * 8;
  page = page + GL(dl_pagesize) - size;
  while (1)
    {
      if (__mprotect ((void *) page, size,
		      PROT_READ|PROT_WRITE|PROT_EXEC) == 0)
	/* We got this chunk changed; loop to do another chunk below.  */
	page -= size;
      else
	{
	  if (errno != ENOMEM)	/* Unexpected failure mode.  */
	    return errno;

	  if (size == GL(dl_pagesize))
	    /* We just tried to mprotect the top hole page and failed.
	       We are done.  */
	    break;

	  /* Our mprotect call failed because it started below the lowest
	     stack page.  Try again on just the top half of that region.  */
	  size /= 2;
	  page += size;
	}
    }

#elif _STACK_GROWS_UP

  /* This gives us the lowest page that needs to be changed.  */
  uintptr_t page = (uintptr_t) __libc_stack_end & -(intptr_t) GL(dl_pagesize);

  /* There is always a hole in the address space above the top of the
     stack.  So when we make an mprotect call that spans past the top
     of the stack, it will include the hole and fail with ENOMEM.

     We start with a random guess at how deep the stack might have gotten
     so as to have extended the GROWSUP mapping to higher pages.  */

  size_t size = GL(dl_pagesize) * 8;
  while (1)
    {
      if (__mprotect ((void *) page, size,
		      PROT_READ|PROT_WRITE|PROT_EXEC) == 0)
	/* We got this chunk changed; loop to do another chunk below.  */
	page += size;
      else
	{
	  if (errno != ENOMEM)	/* Unexpected failure mode.  */
	    return errno;

	  if (size == GL(dl_pagesize))
	    /* We just tried to mprotect the lowest hole page and failed.
	       We are done.  */
	    break;

	  /* Our mprotect call failed because it extended past the highest
	     stack page.  Try again on just the bottom half of that region.  */
	  size /= 2;
	}
    }

#else
# error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif

  return 0;
}
rtld_hidden_def (_dl_make_stack_executable)
