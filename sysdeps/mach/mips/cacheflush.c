/* Flush the insn cache after GCC writes a closure on the stack.  Mach/MIPS.
Copyright (C) 1994 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <mach.h>
#include <mach/vm_attributes.h>

/* Stupid name, but this is what GCC generates (config/mips/mips.h).  */
void
cacheflush (void *addr, size_t size, int flag)
{
  vm_machine_attribute_val_t val;

  switch (flag)
    {
    case 0:			/* ? */
      val = MATTR_VAL_DCACHE_FLUSH;
    case 1:			/* This is the only value GCC uses.  */
      val = MATTR_VAL_ICACHE_FLUSH;
      break;
    default:
      val = MATTR_VAL_CACHE_FLUSH;
    }

  __vm_machine_attribute (__mach_task_self (),
			  (vm_address_t) addr, size,
			  MATTR_CACHE,
			  &val);
}
