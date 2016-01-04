/* brk -- Adjust the "break" at the end of initial data.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <errno.h>
#include <libc-internal.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <unistd.h>

/* sbrk.c expects this.  */
void *__curbrk;

static uintptr_t
page_above (void *addr)
{
  return ALIGN_UP ((uintptr_t) addr, EXEC_PAGESIZE);
}

/* Set the end of the process's data space to ADDR.
   Return 0 if successful, -1 if not.  */
int
__brk (void *addr)
{
  /* The NaCl sysbrk call is deprecated, so we do not use it here.  Other
     libc code expects that __sbrk can be used at least a little bit, so
     rather than a plain stub we have a minimal __brk implementation here.
     It just uses mmap/munmap to grow or shrink the break area, punting as
     soon as mmap fails to use the same contiguous area.  */

  if (__glibc_unlikely (__curbrk == NULL))
    {
      /* This is the first call.  We must initialize the record
         of the current position.  It starts out at the end of the
         main program's data segment.  */

      /* XXX dynamic case??? */
      extern char _end[];
      __curbrk = _end;
    }

  if (__glibc_unlikely (addr == NULL))
    /* This is a call just to ensure that __curbrk is set up.  */
    return 0;

  uintptr_t old_limit = page_above (__curbrk);
  uintptr_t new_limit = page_above (addr);

  if (old_limit > new_limit)
    {
      /* We're shrinking the old heap enough to release some pages.  */
      if (__munmap ((void *) new_limit, old_limit - new_limit) != 0)
	return -1;
    }
  else if (old_limit < new_limit)
    {
      /* We're growing the old heap enough to need some more pages.
	 See if they are available.  */
      void *new_space = __mmap ((void *) old_limit, new_limit - old_limit,
				PROT_READ | PROT_WRITE, MAP_ANON, -1, 0);
      if (new_space != (void *) old_limit)
	{
          if (new_space != MAP_FAILED)
            {
              /* mmap chose some different place for the pages
                 because the contiguous area was not available.
                 Oh well.  We can't use that.  */
              __munmap (new_space, new_limit - old_limit);
              __set_errno (ENOMEM);
            }
	  return -1;
	}
    }

  __curbrk = addr;
  return 0;
}
weak_alias (__brk, brk)
