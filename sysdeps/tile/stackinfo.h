/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

/* This file contains a bit of information about the stack allocation
   of the processor.  */

#ifndef _STACKINFO_H
#define _STACKINFO_H	1

#include <elf.h>

/* On tile the stack grows down.  */
#define _STACK_GROWS_DOWN	1

/* Default to a non-executable stack. */
#define DEFAULT_STACK_PERMS (PF_R|PF_W)

/* Access to the stack pointer.  The macros are used in alloca_account
   for which they need to act as barriers as well, hence the additional
   (unnecessary) parameters.  */
#define stackinfo_get_sp() \
  ({ void *p__; asm volatile ("move %0, sp" : "=r" (p__)); p__; })
#if defined __tilegx__ && __WORDSIZE == 32
#define __stackinfo_sub "subx"
#else
#define __stackinfo_sub "sub"
#endif
#define stackinfo_sub_sp(ptr)                                  \
  ({ ptrdiff_t d__;                                             \
    asm volatile (__stackinfo_sub " %0, %0, sp" : "=r" (d__) : "0" (ptr));  \
    d__; })

#endif /* stackinfo.h */
