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

ENTRY(_savef_all)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef14)
C_TEXT(_savef14):	stfd	fp14,-144(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef15)
C_TEXT(_savef15):	stfd	fp15,-136(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef16)
C_TEXT(_savef16):	stfd	fp16,-128(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef17)
C_TEXT(_savef17):	stfd	fp17,-120(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef18)
C_TEXT(_savef18):	stfd	fp18,-112(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef19)
C_TEXT(_savef19):	stfd	fp19,-104(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef20)
C_TEXT(_savef20):	stfd	fp20,-96(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef21)
C_TEXT(_savef21):	stfd	fp21,-88(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef22)
C_TEXT(_savef22):	stfd	fp22,-80(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef23)
C_TEXT(_savef23):	stfd	fp23,-72(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef24)
C_TEXT(_savef24):	stfd	fp24,-64(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef25)
C_TEXT(_savef25):	stfd	fp25,-56(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef26)
C_TEXT(_savef26):	stfd	fp26,-48(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef27)
C_TEXT(_savef27):	stfd	fp27,-40(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef28)
C_TEXT(_savef28):	stfd	fp28,-32(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef29)
C_TEXT(_savef29):	stfd	fp29,-24(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef30)
C_TEXT(_savef30):	stfd	fp30,-16(r11)
		ASM_GLOBAL_DIRECTIVE C_TEXT(_savef31)
C_TEXT(_savef31):	stfd	fp31,-8(r11)
		blr
