/* Flush the insn cache after GCC writes a closure on the stack.  Mach/MIPS.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#include <mach.h>
#include <mach/vm_attributes.h>

/* Stupid name, but this is what GCC generates (config/mips/mips.h).  */
void
cacheflush (void *addr, unsigned size, int flag)
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
