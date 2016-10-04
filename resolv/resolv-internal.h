/* libresolv interfaces for internal use across glibc.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#ifndef _RESOLV_INTERNAL_H
#define _RESOLV_INTERNAL_H 1

#include <resolv.h>
#include <stdbool.h>

/* Internal version of RES_USE_INET6 which does not trigger a
   deprecation warning.  */
#define DEPRECATED_RES_USE_INET6 0x00002000

static inline bool
res_use_inet6 (void)
{
  return _res.options & DEPRECATED_RES_USE_INET6;
}

#endif  /* _RESOLV_INTERNAL_H */
