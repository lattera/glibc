/* System-specific settings for dynamic linker code.  i386 version.
   Copyright (C) 2002,2003,2008,2009 Free Software Foundation, Inc.
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

#ifndef _DL_SYSDEP_H
# include "i686/dl-sysdep.h"

/* sysenter/syscall is not useful on i386 through i586, but the dynamic
   linker and dl code in libc.a has to be able to load i686 compiled
   libraries.  */
# undef USE_DL_SYSINFO

#endif	/* dl-sysdep.h */
