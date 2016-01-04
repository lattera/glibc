/* DL_SYSDEP_OSCHECK macro for NaCl.
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

#ifndef _DL_OSINFO_H
#define _DL_OSINFO_H  1

#include <sysdeps/generic/dl-osinfo.h>

#include "nacl-interfaces.h"

#ifndef SHARED
/* This doesn't really have anything to do with the purpose for
   which this macro is used in Linux configurations.  But it is
   called at the right place in __libc_start_main.  */
# define DL_SYSDEP_OSCHECK(fatal)	__nacl_initialize_interfaces ()
#endif


#endif /* dl-osinfo.h */
