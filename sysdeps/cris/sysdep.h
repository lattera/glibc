/* Assembler macros for CRIS.
   Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdeps/generic/sysdep.h>

#ifndef HAVE_ELF
# error ELF is assumed.  Generalize the code and retry.
#endif

#ifndef NO_UNDERSCORES
# error User-label prefix (underscore) assumed absent.  Generalize the code and retry.
#endif

#ifdef	__ASSEMBLER__

/* Syntactic details of assembly-code.  */

/* It is *not* generally true that "ELF uses byte-counts for .align, most
   others use log2 of count of bytes", like some neighboring configs say.
   See "align" in gas/read.c which is not overridden by
   gas/config/obj-elf.c.  It takes a log2 argument.  *Some* targets
   override it to take a byte argument.  People should read source instead
   of relying on hearsay.  */
# define ALIGNARG(log2) log2

# define ASM_TYPE_DIRECTIVE(name,typearg) .type name,typearg
# define ASM_SIZE_DIRECTIVE(name) .size name,.-name

/* The non-PIC jump is preferred, since it does not stall, and does not
   invoke generation of a PLT.  These macros assume that $r0 is set up as
   GOT register.  */
# ifdef __PIC__
#  define PLTJUMP(_x) \
  add.d	C_SYMBOL_NAME (_x):PLT,$pc

#  define PLTCALL(_x) \
  move.d C_SYMBOL_NAME (_x):PLTG,$r9			@ \
  add.d	$r0,$r9						@ \
  jsr	$r9

#  define SETUP_PIC \
  push	$r0						@ \
  move.d $pc,$r0					@ \
  sub.d	.:GOTOFF,$r0

#  define TEARDOWN_PIC pop $r0
# else
#  define PLTJUMP(_x) jump C_SYMBOL_NAME (_x)
#  define PLTCALL(_x) jsr  C_SYMBOL_NAME (_x)
#  define SETUP_PIC
#  define TEARDOWN_PIC
# endif

/* Define an entry point visible from C.  */
# define ENTRY(name) \
  .text							@ \
  ASM_GLOBAL_DIRECTIVE C_SYMBOL_NAME (name) 		@ \
  ASM_TYPE_DIRECTIVE (C_SYMBOL_NAME (name), function)	@ \
  .align ALIGNARG (2) 					@ \
  C_LABEL(name)						@ \
  CALL_MCOUNT

# undef	END
# define END(name) \
  ASM_SIZE_DIRECTIVE (C_SYMBOL_NAME (name))

/* If compiled for profiling, call `mcount' at the start of each function.
   FIXME: Note that profiling is not actually implemented.  This is just
   example code which might not even compile, though it is believed to be
   correct.  */
# ifdef	PROF
#  define CALL_MCOUNT \
  push	$srp						@ \
  push	$r9						@ \
  push	$r10						@ \
  push	$r11						@ \
  push	$r12						@ \
  push	$r13						@ \
  SETUP_PIC						@ \
  PLTCALL (mcount)					@ \
  TEARDOWN_PIC						@ \
  pop	$r13						@ \
  pop	$r12						@ \
  pop	$r11						@ \
  pop	$r10						@ \
  pop	$r9						@ \
  pop	$srp
# else
#  define CALL_MCOUNT		/* Do nothing.  */
# endif

/* Since C identifiers are not normally prefixed with an underscore
   on this system, the asm identifier `syscall_error' intrudes on the
   C name space.  Make sure we use an innocuous name.  */
# define syscall_error	__syscall_error
# define mcount		_mcount

#endif	/* __ASSEMBLER__ */
