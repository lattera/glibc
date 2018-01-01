/* Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

#ifndef _CALLER_H
#define _CALLER_H 1

#include <ldsodefs.h>

/* _dl_check_caller only works in DSOs.  */
#ifdef SHARED
# define __check_caller(caller, mask) \
  GLRO(dl_check_caller) (caller, mask)
#else
# define __check_caller(caller, mask) (0)
#endif

#endif /* caller.h */
