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

		ASM_GLOBAL_DIRECTIVE _restgpr0_13
_restgpr0_13:	lwz	r13,-76(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_14
_restgpr0_14:	lwz	r14,-72(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_15
_restgpr0_15:	lwz	r15,-68(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_16
_restgpr0_16:	lwz	r16,-64(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_17
_restgpr0_17:	lwz	r17,-60(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_18
_restgpr0_18:	lwz	r18,-56(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_19
_restgpr0_19:	lwz	r19,-52(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_20
_restgpr0_20:	lwz	r20,-48(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_21
_restgpr0_21:	lwz	r21,-44(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_22
_restgpr0_22:	lwz	r22,-40(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_23
_restgpr0_23:	lwz	r23,-36(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_24
_restgpr0_24:	lwz	r24,-32(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_25
_restgpr0_25:	lwz	r25,-28(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_26
_restgpr0_26:	lwz	r26,-24(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_27
_restgpr0_27:	lwz	r27,-20(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_28
_restgpr0_28:	lwz	r28,-16(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_29
_restgpr0_29:	lwz	r29,-12(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_30
_restgpr0_30:	lwz	r30,-8(r1)
		ASM_GLOBAL_DIRECTIVE _restgpr0_31
_restgpr0_31:	lwz	r31,-4(r1)
		blr
