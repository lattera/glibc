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

		ASM_GLOBAL_DIRECTIVE _restfpr_14
_restfpr_14:	lfd	fp14,-144(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_15
_restfpr_15:	lfd	fp15,-136(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_16
_restfpr_16:	lfd	fp16,-128(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_17
_restfpr_17:	lfd	fp17,-120(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_18
_restfpr_18:	lfd	fp18,-112(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_19
_restfpr_19:	lfd	fp19,-104(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_20
_restfpr_20:	lfd	fp20,-96(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_21
_restfpr_21:	lfd	fp21,-88(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_22
_restfpr_22:	lfd	fp22,-80(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_23
_restfpr_23:	lfd	fp23,-72(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_24
_restfpr_24:	lfd	fp24,-64(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_25
_restfpr_25:	lfd	fp25,-56(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_26
_restfpr_26:	lfd	fp26,-48(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_27
_restfpr_27:	lfd	fp27,-40(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_28
_restfpr_28:	lfd	fp28,-32(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_29
_restfpr_29:	lfd	fp29,-24(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_30
_restfpr_30:	lfd	fp30,-16(r11)
		ASM_GLOBAL_DIRECTIVE _restfpr_31
_restfpr_31:	lfd	fp31,-8(r11)
		blr
