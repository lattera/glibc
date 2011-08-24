/* Copyright (C) 2005, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller <davem@davemloft.net>, 2005.

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

#include <setjmp.h>
#include <stdint.h>
#include <unwind.h>

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address, demangle)			\
  ((unsigned long int) (address) < (jmpbuf)->uc_mcontext.mc_gregs[MC_O6] + 2047)

#define _JMPBUF_CFA_UNWINDS_ADJ(_jmpbuf, _context, _adj) \
  _JMPBUF_UNWINDS_ADJ (_jmpbuf, (void *) _Unwind_GetCFA (_context), _adj)

#define _JMPBUF_UNWINDS_ADJ(_jmpbuf, _address, _adj) \
  ((uintptr_t) (_address) - (_adj) \
   < (uintptr_t) (_jmpbuf)[0].uc_mcontext.mc_gregs[MC_O6] + 2047 - (_adj))

/* We use the normal lobngjmp for unwinding.  */
#define __libc_unwind_longjmp(buf, val) __libc_longjmp (buf, val)
