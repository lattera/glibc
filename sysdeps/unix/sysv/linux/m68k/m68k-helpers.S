/* Copyright (C) 2010-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   In addition to the permissions in the GNU Lesser General Public
   License, the Free Software Foundation gives you unlimited
   permission to link the compiled version of this file with other
   programs, and to distribute those programs without any restriction
   coming from the use of this file. (The GNU Lesser General Public
   License restrictions do apply in other respects; for example, they
   cover modification of the file, and distribution when not linked
   into another program.)

   Note that people who make modified versions of this file are not
   obligated to grant this special exception for their modified
   versions; it is their choice whether to do so. The GNU Lesser
   General Public License gives permission to release a modified
   version without this exception; this exception also makes it
   possible to release a modified version which carries forward this
   exception.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <bits/m68k-vdso.h>

	.text

	.hidden __vdso_read_tp_stub
ENTRY (__vdso_read_tp_stub)
	move.l	#__NR_get_thread_area, %d0
	trap	#0
	move.l	%d0, %a0
	rts
END (__vdso_read_tp_stub)

# ifdef SHARED
/* GCC will emit calls to this routine.  Linux has an
   equivalent helper function (which clobbers fewer registers than
   a normal function call) in a vdso; tail call to the
   helper.  */
# if IS_IN (rtld)
/* rtld gets a hidden copy of __m68k_read_tp.  */
	.hidden __m68k_read_tp
# endif
ENTRY (__m68k_read_tp)
	LOAD_GOT (%a0)
	move.l	M68K_VDSO_SYMBOL (__vdso_read_tp)@GOT(%a0), %a0
	move.l	(%a0), %a0
	jmp	(%a0)
END (__m68k_read_tp)

/* The following two stubs are for macros in atomic.h, they can't
   clobber anything.  */

	.hidden __vdso_atomic_cmpxchg_32_stub
ENTRY (__vdso_atomic_cmpxchg_32_stub)
	move.l	%d2, -(%sp)
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%d2, 0)
	move.l	%d0, %d2
	move.l	#SYS_ify (atomic_cmpxchg_32), %d0
	trap	#0
	move.l	(%sp)+, %d2
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d2)
	rts
END (__vdso_atomic_cmpxchg_32_stub)

	.hidden __vdso_atomic_barrier_stub
ENTRY (__vdso_atomic_barrier_stub)
	move.l	%d0, -(%sp)
	cfi_adjust_cfa_offset (4)
	move.l	#SYS_ify (atomic_barrier), %d0
	trap	#0
	move.l	(%sp)+, %d0
	cfi_adjust_cfa_offset (-4)
	rts
END (__vdso_atomic_barrier_stub)
# else /* !SHARED */
/* If the vDSO is not available, use a syscall to get TP.  */
	strong_alias (__vdso_read_tp_stub, __m68k_read_tp)
# endif /* SHARED */
