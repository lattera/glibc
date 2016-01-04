/* getsysstats - Determine various system internal values.  NaCl version.
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
#include <sys/sysinfo.h>
#include <nacl-interfaces.h>

#undef __native_client__
#include "native_client/src/trusted/service_runtime/include/sys/unistd.h"


int
__get_nprocs_conf (void)
{
  int nprocs;
  if (__nacl_irt_basic.sysconf (NACL_ABI__SC_NPROCESSORS_ONLN, &nprocs) != 0)
    /* On failure (which should be impossible), just report one processor.  */
    nprocs = 1;
  return nprocs;
}
weak_alias (__get_nprocs_conf, get_nprocs_conf)

int
__get_nprocs (void)
{
  return __get_nprocs_conf ();
}
weak_alias (__get_nprocs, get_nprocs)


long int
__get_phys_pages (void)
{
  /* We have no general way to determine this value.  */
  __set_errno (ENOSYS);
  return -1;
}
weak_alias (__get_phys_pages, get_phys_pages)
stub_warning (get_phys_pages)


long int
__get_avphys_pages (void)
{
  /* We have no general way to determine this value.  */
  __set_errno (ENOSYS);
  return -1;
}
weak_alias (__get_avphys_pages, get_avphys_pages)
stub_warning (get_avphys_pages)
