/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <inttypes.h>

/* For Linux/x86-64 we have one extra requirement: the stack must be
   in the first 4GB.  Otherwise the segment register base address is
   not wide enough.  */
#define EXTRA_PARAM_CHECKS \
  if ((uintptr_t) stackaddr > 0x100000000ul)				      \
    /* We cannot handle that stack address.  */				      \
    return EINVAL

#include <nptl/sysdeps/pthread/pthread_attr_setstackaddr.c>
