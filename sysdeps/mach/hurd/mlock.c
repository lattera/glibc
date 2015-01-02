/* mlock -- guarantee pages are resident in memory.  Mach/Hurd version.
   Copyright (C) 2001-2015 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <hurd.h>
#include <mach/mach_host.h>

/* Guarantee all whole pages mapped by the range [ADDR,ADDR+LEN) to
   be memory resident.  */

int
mlock (const void *addr, size_t len)
{
  mach_port_t hostpriv;
  vm_address_t page;
  error_t err;

  err = __get_privileged_ports (&hostpriv, NULL);
  if (err)
    return __hurd_fail (EPERM);

  page = trunc_page ((vm_address_t) addr);
  len = round_page ((vm_address_t) addr + len) - page;
  err = __vm_wire (hostpriv, __mach_task_self (), page, len,
		   VM_PROT_READ);
  __mach_port_deallocate (__mach_task_self (), hostpriv);

  return err ? __hurd_fail (err) : 0;
}
