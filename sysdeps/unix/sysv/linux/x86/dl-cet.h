/* Linux/x86 CET initializers function.
   Copyright (C) 2018 Free Software Foundation, Inc.

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

static inline int __attribute__ ((always_inline))
dl_cet_allocate_legacy_bitmap (unsigned long *legacy_bitmap)
{
  /* FIXME: Need syscall support.  */
  return -1;
}

static inline int __attribute__ ((always_inline))
dl_cet_disable_cet (unsigned int cet_feature)
{
  /* FIXME: Need syscall support.  */
  return -1;
}

static inline int __attribute__ ((always_inline))
dl_cet_lock_cet (void)
{
  /* FIXME: Need syscall support.  */
  return -1;
}
