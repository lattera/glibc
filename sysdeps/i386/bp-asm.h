/* Bounded-pointer definitions for x86 assembler.
   Copyright (C) 2000-2013 Free Software Foundation, Inc.
   Contributed by Greg McGary <greg@mcgary.org>
   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in the GNU MP Library.

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

#ifndef _bp_asm_h_
# define _bp_asm_h_ 1

# if __ASSEMBLER__

#  if __BOUNDED_POINTERS__

/* Bounded pointers occupy three words.  */
#   define PTR_SIZE 12
/* Bounded pointer return values are passed back through a hidden
   argument that points to caller-allocate space.  The hidden arg
   occupies one word on the stack.  */
#   define RTN_SIZE 4
/* Although the caller pushes the hidden arg, the callee is
   responsible for popping it.  */
#   define RET_PTR ret $RTN_SIZE
/* Stack space overhead of procedure-call linkage: return address and
   frame pointer.  */
#   define LINKAGE 8
/* Stack offset of return address after calling ENTER.  */
#   define PCOFF 4

#  else /* !__BOUNDED_POINTERS__ */

/* Unbounded pointers occupy one word.  */
#   define PTR_SIZE 4
/* Unbounded pointer return values are passed back in the register %eax.  */
#   define RTN_SIZE 0
/* Use simple return instruction for unbounded pointer values.  */
#   define RET_PTR ret
/* Stack space overhead of procedure-call linkage: return address only.  */
#   define LINKAGE 4
/* Stack offset of return address after calling ENTER.  */
#   define PCOFF 0

#  endif /* !__BOUNDED_POINTERS__ */

# endif /* __ASSEMBLER__ */

#endif /* _bp_asm_h_ */
