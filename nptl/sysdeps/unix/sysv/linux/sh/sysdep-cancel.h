/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if !defined NOT_IN_libc || defined IS_IN_libpthread || defined IS_IN_librt

# define _IMM12 #-12
# define _IMM16 #-16
# define _IMP16 #16
# undef PSEUDO
# define PSEUDO(name, syscall_name, args) \
  .text; \
  ENTRY (name); \
  .Lpseudo_start: \
    SINGLE_THREAD_P; \
    bf .Lpseudo_cancel; \
    .type __##syscall_name##_nocancel,@function; \
    .globl __##syscall_name##_nocancel; \
    __##syscall_name##_nocancel: \
    DO_CALL (syscall_name, args); \
    mov r0,r1; \
    mov _IMM12,r2; \
    shad r2,r1; \
    not r1,r1; \
    tst r1,r1; \
    bt .Lsyscall_error; \
    bra .Lpseudo_end; \
     nop; \
    .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel; \
 .Lpseudo_cancel: \
    sts.l pr,@-r15; \
 .LCFI0: \
    add _IMM16,r15; \
    SAVE_ARGS_##args; \
 .LCFI1: \
    CENABLE; \
    LOAD_ARGS_##args; \
    add _IMP16,r15; \
 .LCFI2: \
    lds.l @r15+,pr; \
 .LCFI3: \
    DO_CALL(syscall_name, args); \
    SYSCALL_INST_PAD; \
    sts.l pr,@-r15; \
 .LCFI4: \
    mov.l r0,@-r15; \
 .LCFI5: \
    CDISABLE; \
    mov.l @r15+,r0; \
 .LCFI6: \
    lds.l @r15+,pr; \
 .LCFI7: \
    mov r0,r1; \
    mov _IMM12,r2; \
    shad r2,r1; \
    not r1,r1; \
    tst r1,r1; \
    bf .Lpseudo_end; \
 .Lsyscall_error: \
    SYSCALL_ERROR_HANDLER; \
 .Lpseudo_end: \
 /* Create unwinding information for the syscall wrapper.  */ \
 .section .eh_frame,"a",@progbits; \
 .Lframe1: \
    .ualong .LECIE1-.LSCIE1; \
 .LSCIE1: \
    .ualong 0x0; \
    .byte   0x1; \
    AUGMENTATION_STRING; \
    .uleb128 0x1; \
    .sleb128 -4; \
    .byte   0x11; \
    AUGMENTATION_PARAM; \
    .byte   0xc; \
    .uleb128 0xf; \
    .uleb128 0x0; \
    .align 2; \
 .LECIE1: \
 .LSFDE1: \
    .ualong .LEFDE1-.LASFDE1; \
 .LASFDE1: \
    .ualong .LASFDE1-.Lframe1; \
    START_SYMBOL_REF; \
    .ualong .Lpseudo_end - .Lpseudo_start; \
    AUGMENTATION_PARAM_FDE; \
    .byte   0x4; \
    .ualong .LCFI0-.Lpseudo_start; \
    .byte   0xe; \
    .uleb128 0x4; \
    .byte   0x91; \
    .uleb128 0x1; \
    .byte   0x4; \
    .ualong .LCFI1-.LCFI0; \
    .byte   0xe; \
    .uleb128 0x14; \
    FRAME_REG_##args; \
    .byte   0x4; \
    .ualong .LCFI2-.LCFI1; \
    .byte   0xe; \
    .uleb128 0x4; \
    .byte   0x4; \
    .ualong .LCFI3-.LCFI2; \
    .byte   0xe; \
    .uleb128 0x0; \
    .byte   0xd1; \
    .byte   0x4; \
    .ualong .LCFI4-.LCFI3; \
    .byte   0xe; \
    .uleb128 0x4; \
    .byte   0x91; \
    .uleb128 0x1; \
    .byte   0x4; \
    .ualong .LCFI5-.LCFI4; \
    .byte   0xe; \
    .uleb128 0x8; \
    .byte   0x80; \
    .uleb128 0x2; \
    .byte   0x4; \
    .ualong .LCFI6-.LCFI5; \
    .byte   0xe; \
    .uleb128 0x4; \
    .byte   0xc0; \
    .byte   0x4; \
    .ualong .LCFI7-.LCFI6; \
    .byte   0xe; \
    .uleb128 0x0; \
    .byte   0xd1; \
    .align 2; \
 .LEFDE1: \
 .previous

# ifdef SHARED
#  define AUGMENTATION_STRING .string "zR"
#  define AUGMENTATION_PARAM .uleb128 1; .byte 0x1b
#  define AUGMENTATION_PARAM_FDE .uleb128 0
#  define START_SYMBOL_REF .long .Lpseudo_start-.
# else
#  define AUGMENTATION_STRING .ascii "\0"
#  define AUGMENTATION_PARAM
#  define AUGMENTATION_PARAM_FDE
#  define START_SYMBOL_REF .long .Lpseudo_start
# endif

# define FRAME_REG_0	/* Nothing.  */
# define FRAME_REG_1	FRAME_REG_0; .byte 0x84; .uleb128 5
# define FRAME_REG_2	FRAME_REG_1; .byte 0x85; .uleb128 4
# define FRAME_REG_3	FRAME_REG_2; .byte 0x86; .uleb128 3
# define FRAME_REG_4	FRAME_REG_3; .byte 0x87; .uleb128 2
# define FRAME_REG_5	FRAME_REG_4
# define FRAME_REG_6	FRAME_REG_5

# undef PSEUDO_END
# define PSEUDO_END(sym) \
  END (sym)

# define SAVE_ARGS_0	/* Nothing.  */
# define SAVE_ARGS_1	SAVE_ARGS_0; mov.l r4,@(0,r15)
# define SAVE_ARGS_2	SAVE_ARGS_1; mov.l r5,@(4,r15)
# define SAVE_ARGS_3	SAVE_ARGS_2; mov.l r6,@(8,r15)
# define SAVE_ARGS_4	SAVE_ARGS_3; mov.l r7,@(12,r15)
# define SAVE_ARGS_5	SAVE_ARGS_4
# define SAVE_ARGS_6	SAVE_ARGS_5

# define LOAD_ARGS_0	/* Nothing.  */
# define LOAD_ARGS_1	LOAD_ARGS_0; mov.l @(0,r15),r4
# define LOAD_ARGS_2	LOAD_ARGS_1; mov.l @(4,r15),r5
# define LOAD_ARGS_3	LOAD_ARGS_2; mov.l @(8,r15),r6
# define LOAD_ARGS_4	LOAD_ARGS_3; mov.l @(12,r15),r7
# define LOAD_ARGS_5	LOAD_ARGS_4
# define LOAD_ARGS_6	LOAD_ARGS_5

# ifdef IS_IN_libpthread
#  define __local_enable_asynccancel	__pthread_enable_asynccancel
#  define __local_disable_asynccancel	__pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define __local_enable_asynccancel	__libc_enable_asynccancel
#  define __local_disable_asynccancel	__libc_disable_asynccancel
# elif defined IS_IN_librt
#  define __local_enable_asynccancel	__librt_enable_asynccancel
#  define __local_disable_asynccancel	__librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# define CENABLE \
	mov.l 1f,r0; \
	bsrf r0; \
	 nop; \
     0: bra 2f; \
	 mov r0,r2; \
	.align 2; \
     1: .long __local_enable_asynccancel - 0b; \
     2:

# define CDISABLE \
	mov.l 1f,r0; \
	bsrf r0; \
	 mov r2,r4; \
     0: bra 2f; \
	 nop; \
	.align 2; \
     1: .long __local_disable_asynccancel - 0b; \
     2:

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P \
	stc gbr,r0; \
	mov.w 0f,r1; \
	sub r1,r0; \
	mov.l @(MULTIPLE_THREADS_OFFSET,r0),r0; \
	bra 1f; \
	 tst r0,r0; \
     0: .word TLS_PRE_TCB_SIZE; \
     1:

# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif
