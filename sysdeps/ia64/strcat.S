/* IA-64 assembly version of the standard strcat() function.  
   This file is part of the GNU C Library.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Dan Pop <Dan.Pop@cern.ch>.

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

/* Return: dest

   Inputs:
        in0:    dest
        in1:    src

   A straightforword implementation: strcpy(dest + strlen(dest), src).

   Could be marginally optimised by inlining the code of strcpy() and
   strlen(), to avoid the two function calls.  OTOH, by not doing that,
   we avoid L1I cache pollution (code using strcat() is likely to also
   use strcpy() and strlen(), so we already have a copy of these functions
   in the cache).  */

#include <sysdep.h>
#undef ret

#define src	in1
#define dest	in0
#define save_b0	loc0
#define save_pfs loc1
#define tmp	loc2
#define rc	ret0

ENTRY(strcat)
	.prologue ASM_UNW_PRLG_RP|ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE(2)
	alloc 	save_pfs = ar.pfs, 2, 3, 2, 0
	mov	save_b0 = b0
	.body
	mov	out0 = dest
	mov	tmp = gp ;;
	br.call.sptk.many b0 = strlen# ;; // rc = strlen(dest);
        mov	gp = tmp
	add	out0 = dest, rc
	mov	out1 = src
	br.call.sptk.many b0 = strcpy# ;; // strcpy(dest + strlen(dest), src)
	mov     gp = tmp
	mov	rc = dest
	mov	b0 = save_b0
	mov	ar.pfs = save_pfs
	br.ret.sptk.many b0
END(strcat)
