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

ENTRY(_restf_all)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf14)
C_TEXT(_restf14):	lfd	fp14,-144(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf15)
C_TEXT(_restf15):	lfd	fp15,-136(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf16)
C_TEXT(_restf16):	lfd	fp16,-128(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf17)
C_TEXT(_restf17):	lfd	fp17,-120(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf18)
C_TEXT(_restf18):	lfd	fp18,-112(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf19)
C_TEXT(_restf19):	lfd	fp19,-104(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf20)
C_TEXT(_restf20):	lfd	fp20,-96(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf21)
C_TEXT(_restf21):	lfd	fp21,-88(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf22)
C_TEXT(_restf22):	lfd	fp22,-80(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf23)
C_TEXT(_restf23):	lfd	fp23,-72(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf24)
C_TEXT(_restf24):	lfd	fp24,-64(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf25)
C_TEXT(_restf25):	lfd	fp25,-56(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf26)
C_TEXT(_restf26):	lfd	fp26,-48(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf27)
C_TEXT(_restf27):	lfd	fp27,-40(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf28)
C_TEXT(_restf28):	lfd	fp28,-32(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf29)
C_TEXT(_restf29):	lfd	fp29,-24(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf30)
C_TEXT(_restf30):	lfd	fp30,-16(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_restf31)
C_TEXT(_restf31):	lfd	fp31,-8(r11)
		blr
