/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

		ASM_GLOBAL_DIRECTIVE _savegpr0_13
_savegpr0_13:	stw	r13,-76(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_14
_savegpr0_14:	stw	r14,-72(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_15
_savegpr0_15:	stw	r15,-68(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_16
_savegpr0_16:	stw	r16,-64(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_17
_savegpr0_17:	stw	r17,-60(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_18
_savegpr0_18:	stw	r18,-56(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_19
_savegpr0_19:	stw	r19,-52(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_20
_savegpr0_20:	stw	r20,-48(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_21
_savegpr0_21:	stw	r21,-44(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_22
_savegpr0_22:	stw	r22,-40(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_23
_savegpr0_23:	stw	r23,-36(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_24
_savegpr0_24:	stw	r24,-32(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_25
_savegpr0_25:	stw	r25,-28(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_26
_savegpr0_26:	stw	r26,-24(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_27
_savegpr0_27:	stw	r27,-20(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_28
_savegpr0_28:	stw	r28,-16(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_29
_savegpr0_29:	stw	r29,-12(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_30
_savegpr0_30:	stw	r30,-8(r1)
		ASM_GLOBAL_DIRECTIVE _savegpr0_31
_savegpr0_31:	stw	r31,-4(r1)
		stw	r0,8(r1)
		blr
