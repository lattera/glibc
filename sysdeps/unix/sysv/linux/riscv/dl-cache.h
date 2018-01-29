/* Support for reading /etc/ld.so.cache files written by Linux ldconfig.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <ldconfig.h>

/* For now we only support the natural XLEN ABI length on all targets, so the
   only bits that need to go into ld.so.cache are the FLEG ABI length.  */
#if defined __riscv_float_abi_double
# define _DL_CACHE_DEFAULT_ID    (FLAG_RISCV_FLOAT_ABI_DOUBLE | FLAG_ELF_LIBC6)
#else
# define _DL_CACHE_DEFAULT_ID    (FLAG_RISCV_FLOAT_ABI_SOFT | FLAG_ELF_LIBC6)
#endif

#define _dl_cache_check_flags(flags)		    			\
  ((flags) == _DL_CACHE_DEFAULT_ID)

/* If given a path to one of our library directories, adds every library
   directory via add_dir (), otherwise just adds the giver directory.  On
   RISC-V, libraries can be found in paths ending in:
     - /lib64/lp64d
     - /lib64/lp64
     - /lib (only ld.so)
   so this will add all of those paths.

   According to Joseph Myers:
       My reasoning for that would be: generic autoconf-configured (etc.)
       software may only know about using the lib directory, so you want the
       lib directory to be searched regardless of the ABI - but it's also
       useful to be able to e.g. list /usr/local/lib in /etc/ld.so.conf for all
       architectures and have that automatically imply /usr/local/lib64/lp64d
       etc. so that libraries can be found that come from software that does
       use the ABI-specific directories.  */
#define add_system_dir(dir) 						\
  do							    		\
    {									\
      size_t len = strlen (dir);					\
      char path[len + 9];						\
      memcpy (path, dir, len + 1);					\
      if (len >= 12 && ! memcmp(path + len - 12, "/lib64/lp64d", 12))	\
	{								\
	  len -= 8;							\
	  path[len] = '\0';						\
	}								\
      if (len >= 11 && ! memcmp(path + len - 11, "/lib64/lp64", 11))	\
	{								\
	  len -= 7;							\
	  path[len] = '\0';						\
	}								\
      add_dir (path);							\
      if (len >= 4 && ! memcmp(path + len - 4, "/lib", 4))		\
	{								\
	  memcpy (path + len, "64/lp64d", 9);				\
	  add_dir (path);						\
	  memcpy (path + len, "64/lp64", 8);				\
	  add_dir (path);						\
	}								\
    } while (0)


#include_next <dl-cache.h>
