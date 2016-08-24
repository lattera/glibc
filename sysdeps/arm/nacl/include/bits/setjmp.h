/* Private jmp_buf-related definitions.  NaCl/ARM version.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _INCLUDE_BITS_SETJMP_H
#define _INCLUDE_BITS_SETJMP_H 1

#ifndef __ASSEMBLER__
/* Get the public declarations.  */
# include <sysdeps/arm/bits/setjmp.h>
#endif

# ifndef _ISOMAC

/* Register list for a ldm/stm instruction to load/store
   the general registers from a __jmp_buf.

   The generic ARM definition includes r9 (v6), which is not
   permitted under NaCl.  We add r3 even though it's call-clobbered,
   just to keep the size the same as the generic version.  */
#define JMP_BUF_REGLIST		{r3, v1-v5, sl, fp}

/* Index of __jmp_buf where the sp register resides.  */
#define __JMP_BUF_SP		0

# endif /* _ISOMAC */
#endif  /* include/bits/setjmp.h */
