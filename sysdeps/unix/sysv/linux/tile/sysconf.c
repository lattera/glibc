/* Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sys/sysinfo.h>
#include <arch/chip.h>

static long int linux_sysconf (int name);

/* Get the value of the system variable NAME.  */
long int
__sysconf (int name)
{
  /* Currently we support only tilepro and tilegx, which have
     statically-known cache sizes.  */
  switch (name)
    {
    /* Level 1 cache.  */
    case _SC_LEVEL1_ICACHE_SIZE:
      return CHIP_L1I_CACHE_SIZE();
    case _SC_LEVEL1_ICACHE_ASSOC:
      return CHIP_L1I_ASSOC();
    case _SC_LEVEL1_ICACHE_LINESIZE:
      return CHIP_L1I_LINE_SIZE();
    case _SC_LEVEL1_DCACHE_SIZE:
      return CHIP_L1D_CACHE_SIZE();
    case _SC_LEVEL1_DCACHE_ASSOC:
      return CHIP_L1D_ASSOC();
    case _SC_LEVEL1_DCACHE_LINESIZE:
      return CHIP_L1D_LINE_SIZE();

    /* Level 2 cache.  */
    case _SC_LEVEL2_CACHE_SIZE:
      return CHIP_L2_CACHE_SIZE();
    case _SC_LEVEL2_CACHE_ASSOC:
      return CHIP_L2_ASSOC();
    case _SC_LEVEL2_CACHE_LINESIZE:
      return CHIP_L2_LINE_SIZE();

    /* Level 3 cache is layered on level 2 cache.  */
    case _SC_LEVEL3_CACHE_SIZE:
      return CHIP_L2_CACHE_SIZE() * __get_nprocs();
    case _SC_LEVEL3_CACHE_ASSOC:
      return CHIP_L2_ASSOC();
    case _SC_LEVEL3_CACHE_LINESIZE:
      return CHIP_L2_LINE_SIZE();

    /* No level 4 cache.  */
    case _SC_LEVEL4_CACHE_SIZE:
    case _SC_LEVEL4_CACHE_ASSOC:
    case _SC_LEVEL4_CACHE_LINESIZE:
      return -1;
    }

  return linux_sysconf (name);
}

/* Now the generic Linux version.  */
#undef __sysconf
#define __sysconf static linux_sysconf
#include <sysdeps/unix/sysv/linux/sysconf.c>
