/* System-specific settings for dynamic linker code.  NaCl version.
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

/* No multiple inclusion protection needed here because it's just macros.
   We don't want to use _DL_SYSDEP_H in case we are #include_next'd.  */

#include_next <dl-sysdep.h>

/* We use AT_SYSINFO for a different purpose than Linux does,
   but we too want to store its value.  */
#define NEED_DL_SYSINFO         1
#define DL_SYSINFO_DEFAULT      0

/* sysdeps/arm/dl-sysdep.h defines this but it does not apply to NaCl.  */
#undef  DL_ARGV_NOT_RELRO
