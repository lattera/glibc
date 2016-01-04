/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
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

#ifndef	_LINK_H
# error "Never include <bits/link.h> directly; use <link.h> instead."
#endif

#define __need_int_reg_t
#include <arch/abi.h>


/* Registers for entry into PLT.  */
typedef struct La_tile_regs
{
  __uint_reg_t lr_reg[10];
} La_tile_regs;

/* Return values for calls from PLT.  */
typedef struct La_tile_retval
{
  /* Up to ten registers can be used for a return value (e.g. small struct). */
  __uint_reg_t lrv_reg[10];
} La_tile_retval;


__BEGIN_DECLS

extern ElfW(Addr) la_tile_gnu_pltenter (ElfW(Sym) *__sym, unsigned int __ndx,
                                        uintptr_t *__refcook,
                                        uintptr_t *__defcook,
                                        La_tile_regs *__regs,
                                        unsigned int *__flags,
                                        const char *__symname,
                                        long int *__framesizep);
extern unsigned int la_tile_gnu_pltexit (ElfW(Sym) *__sym, unsigned int __ndx,
                                         uintptr_t *__refcook,
                                         uintptr_t *__defcook,
                                         const La_tile_regs *__inregs,
                                         La_tile_retval *__outregs,
                                         const char *__symname);

__END_DECLS
