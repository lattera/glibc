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

		ASM_GLOBAL_DIRECTIVE _savefpr_14
_savefpr_14:	stfd	fp14,-144(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_15
_savefpr_15:	stfd	fp15,-136(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_16
_savefpr_16:	stfd	fp16,-128(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_17
_savefpr_17:	stfd	fp17,-120(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_18
_savefpr_18:	stfd	fp18,-112(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_19
_savefpr_19:	stfd	fp19,-104(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_20
_savefpr_20:	stfd	fp20,-96(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_21
_savefpr_21:	stfd	fp21,-88(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_22
_savefpr_22:	stfd	fp22,-80(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_23
_savefpr_23:	stfd	fp23,-72(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_24
_savefpr_24:	stfd	fp24,-64(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_25
_savefpr_25:	stfd	fp25,-56(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_26
_savefpr_26:	stfd	fp26,-48(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_27
_savefpr_27:	stfd	fp27,-40(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_28
_savefpr_28:	stfd	fp28,-32(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_29
_savefpr_29:	stfd	fp29,-24(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_30
_savefpr_30:	stfd	fp30,-16(r11)
		ASM_GLOBAL_DIRECTIVE _savefpr_31
_savefpr_31:	stfd	fp31,-8(r11)
		blr
