/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
  L(name##START):							      \
    cmpl $0, %gs:MULTIPLE_THREADS_OFFSET;				      \
    jne L(pseudo_cancel);						      \
  .type __##syscall_name##_nocancel,@function;				      \
  .globl __##syscall_name##_nocancel;					      \
  __##syscall_name##_nocancel:						      \
    DO_CALL (syscall_name, args);					      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
    ret;								      \
  .size __##syscall_name##_nocancel,.-__##syscall_name##_nocancel;	      \
  L(pseudo_cancel):							      \
    CENABLE								      \
    SAVE_OLDTYPE_##args							      \
    PUSHCARGS_##args							      \
    DOCARGS_##args							      \
    movl $SYS_ify (syscall_name), %eax;					      \
    ENTER_KERNEL;							      \
    POPCARGS_##args;							      \
    POPSTATE_##args							      \
    cmpl $-4095, %eax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):							      \
									      \
  /* Create unwinding information for the syscall wrapper.  */		      \
  .section .eh_frame,"a",@progbits;					      \
  L(STARTFRAME):							      \
    /* Length of the CIE.  */						      \
    .long L(ENDCIE)-L(STARTCIE);					      \
  L(STARTCIE):								      \
    /* CIE ID.  */							      \
    .long 0;								      \
    /* Version number.  */						      \
    .byte 1;								      \
    /* NUL-terminated augmentation string.  */				      \
    AUGMENTATION_STRING;						      \
    /* Code alignment factor.  */					      \
    .uleb128 1;								      \
    /* Data alignment factor.  */					      \
    .sleb128 -4;							      \
    /* Return address register column.  */				      \
    .byte 8;								      \
    /* Optional augmentation parameter.  */				      \
    AUGMENTATION_PARAM							      \
    /* Start of the table initialization.  */				      \
    .byte 0xc;			/* DW_CFA_def_cfa */			      \
    .uleb128 4;								      \
    .uleb128 4;								      \
    .byte 0x88;			/* DW_CFA_offset, column 0x8 */		      \
    .uleb128 1;								      \
    .align 4;								      \
  L(ENDCIE):								      \
    /* Length of the FDE.  */						      \
    .long L(ENDFDE)-L(STARTFDE);					      \
  L(STARTFDE):								      \
    /* CIE pointer.  */							      \
    .long L(STARTFDE)-L(STARTFRAME);					      \
    /* Start address of the code.  */					      \
    START_SYMBOL_REF (name);						      \
    /* Length of the code.  */						      \
    .long L(name##END)-L(name##START);					      \
    /* Augmentation data.  */						      \
    AUGMENTATION_PARAM_FDE						      \
    /* The rest of the code depends on the number of parameters the syscall   \
       takes.  */							      \
    EH_FRAME_##args(name);						      \
    .align 4;								      \
  L(ENDFDE):								      \
  .previous

# ifdef SHARED
/* NUL-terminated augmentation string.  Note "z" means there is an
   augmentation value later on.  */
#  define AUGMENTATION_STRING .string "zR"
#  define AUGMENTATION_PARAM \
    /* Augmentation value length.  */					      \
    .uleb128 1;								      \
    /* Encoding: DW_EH_PE_pcrel + DW_EH_PE_sdata4.  */			      \
    .byte 0x1b;
#  define AUGMENTATION_PARAM_FDE \
    /* No augmentation data.  */					      \
    .uleb128 0;
#  define START_SYMBOL_REF(name) \
    /* PC-relative start address of the code.  */			      \
    .long L(name##START)-.
# else
/* No augmentation.  */
#  define AUGMENTATION_STRING .ascii "\0"
#  define AUGMENTATION_PARAM /* nothing */
#  define AUGMENTATION_PARAM_FDE /* nothing */
#  define START_SYMBOL_REF(name) \
    /* Absolute start address of the code.  */				      \
    .long L(name##START)
# endif

/* Callframe description for syscalls without parameters.  This is very
   simple.  The only place the stack pointer is changed is when the old
   cancellation state value is saved.  */
# define EH_FRAME_0(name) \
    .byte 0x40+L(PUSHSTATE)-L(name##START);	/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x40+L(POPSTATE)-L(PUSHSTATE);	/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4

/* For syscalls with one and two parameters the code is the same as for
   those which take no parameter.  */
# define EH_FRAME_1(name) \
    .byte 0x40+L(SAVEBX1)-L(name##START);	/* DW_CFA_advance_loc+N */    \
    .byte 9;					/* DW_CFA_register */	      \
    .uleb128 3;					/* %ebx */		      \
    .uleb128 2;					/* %edx */		      \
    .byte 0x40+L(RESTBX1)-L(SAVEBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(PUSHSTATE)-L(RESTBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x40+L(SAVEBX2)-L(PUSHSTATE);		/* DW_CFA_advance_loc+N */    \
    .byte 9;					/* DW_CFA_register */	      \
    .uleb128 3;					/* %ebx */		      \
    .uleb128 2;					/* %edx */		      \
    .byte 0x40+L(RESTBX2)-L(SAVEBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSTATE)-L(RESTBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4

# define EH_FRAME_2(name) EH_FRAME_1 (name)

/* For syscalls with three parameters the stack pointer is changed
   also to save the content of the %ebx register.  */
# define EH_FRAME_3(name) \
    .byte 0x40+L(PUSHBX1)-L(name##START);	/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 2;								      \
    .byte 0x40+L(POPBX1)-L(PUSHBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4;								      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(PUSHSTATE)-L(POPBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x40+L(PUSHBX2)-L(PUSHSTATE);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 3;								      \
    .byte 0x40+L(POPBX2)-L(PUSHBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSTATE)-L(POPBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4

/* With four parameters the syscall wrappers have to save %ebx and %esi.  */
# define EH_FRAME_4(name) \
    .byte 0x40+L(PUSHSI1)-L(name##START);	/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x86;					/* DW_CFA_offset %esi */      \
    .uleb128 2;								      \
    .byte 0x40+L(PUSHBX1)-L(PUSHSI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 3;								      \
    .byte 0x40+L(POPBX1)-L(PUSHBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSI1)-L(POPBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4;								      \
    .byte 0xc6;					/* DW_CFA_restore %esi */     \
    .byte 0x40+L(PUSHSTATE)-L(POPSI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x40+L(PUSHSI2)-L(PUSHSTATE);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0x86;					/* DW_CFA_offset %esi */      \
    .uleb128 3;								      \
    .byte 0x40+L(PUSHBX2)-L(PUSHSI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 16;							      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 4;								      \
    .byte 0x40+L(POPBX2)-L(PUSHBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSI2)-L(POPBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0xc6;					/* DW_CFA_restore %esi */     \
    .byte 0x40+L(POPSTATE)-L(POPSI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4

/* With five parameters the syscall wrappers have to save %ebx, %esi,
   and %edi.  */
# define EH_FRAME_5(name) \
    .byte 0x40+L(PUSHDI1)-L(name##START);	/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x87;					/* DW_CFA_offset %edi */      \
    .uleb128 2;								      \
    .byte 0x40+L(PUSHSI1)-L(PUSHDI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0x86;					/* DW_CFA_offset %esi */      \
    .uleb128 3;								      \
    .byte 0x40+L(PUSHBX1)-L(PUSHSI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 16;							      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 4;								      \
    .byte 0x40+L(POPBX1)-L(PUSHBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSI1)-L(POPBX1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0xc6;					/* DW_CFA_restore %esi */     \
    .byte 0x40+L(POPDI1)-L(POPSI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4;								      \
    .byte 0xc7;					/* DW_CFA_restore %edi */     \
    .byte 0x40+L(PUSHSTATE)-L(POPDI1);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0x40+L(PUSHDI2)-L(PUSHSTATE);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0x87;					/* DW_CFA_offset %edi */      \
    .uleb128 3;								      \
    .byte 0x40+L(PUSHSI2)-L(PUSHDI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 16;							      \
    .byte 0x86;					/* DW_CFA_offset %esi */      \
    .uleb128 4;								      \
    .byte 0x40+L(PUSHBX2)-L(PUSHSI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 20;							      \
    .byte 0x83;					/* DW_CFA_offset %ebx */      \
    .uleb128 5;								      \
    .byte 0x40+L(POPBX2)-L(PUSHBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 16;							      \
    .byte 0xc3;					/* DW_CFA_restore %ebx */     \
    .byte 0x40+L(POPSI2)-L(POPBX2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 12;							      \
    .byte 0xc6;					/* DW_CFA_restore %esi */     \
    .byte 0x40+L(POPDI2)-L(POPSI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 8;								      \
    .byte 0xc7;					/* DW_CFA_restore %edi */     \
    .byte 0x40+L(POPSTATE)-L(POPDI2);		/* DW_CFA_advance_loc+N */    \
    .byte 14;					/* DW_CFA_def_cfa_offset */   \
    .uleb128 4


# undef ASM_SIZE_DIRECTIVE
# define ASM_SIZE_DIRECTIVE(name) L(name##END): .size name,.-name;

# define SAVE_OLDTYPE_0	movl %eax, %ecx;
# define SAVE_OLDTYPE_1	SAVE_OLDTYPE_0
# define SAVE_OLDTYPE_2	pushl %eax; L(PUSHSTATE):
# define SAVE_OLDTYPE_3	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_4	SAVE_OLDTYPE_2
# define SAVE_OLDTYPE_5	SAVE_OLDTYPE_2

# define PUSHCARGS_0	/* No arguments to push.  */
# define DOCARGS_0	/* No arguments to frob.  */
# define POPCARGS_0	/* No arguments to pop.  */
# define _PUSHCARGS_0	/* No arguments to push.  */
# define _POPCARGS_0	/* No arguments to pop.  */

# define PUSHCARGS_1	movl %ebx, %edx; L(SAVEBX2): PUSHCARGS_0
# define DOCARGS_1	_DOARGS_1 (4)
# define POPCARGS_1	POPCARGS_0; movl %edx, %ebx; L(RESTBX2):
# define _PUSHCARGS_1	pushl %ebx; L(PUSHBX2): _PUSHCARGS_0
# define _POPCARGS_1	_POPCARGS_0; popl %ebx; L(POPBX2):

# define PUSHCARGS_2	PUSHCARGS_1
# define DOCARGS_2	_DOARGS_2 (12)
# define POPCARGS_2	POPCARGS_1
# define _PUSHCARGS_2	_PUSHCARGS_1
# define _POPCARGS_2	_POPCARGS_1

# define PUSHCARGS_3	_PUSHCARGS_2
# define DOCARGS_3	_DOARGS_3 (20)
# define POPCARGS_3	_POPCARGS_3
# define _PUSHCARGS_3	_PUSHCARGS_2
# define _POPCARGS_3	_POPCARGS_2

# define PUSHCARGS_4	_PUSHCARGS_4
# define DOCARGS_4	_DOARGS_4 (28)
# define POPCARGS_4	_POPCARGS_4
# define _PUSHCARGS_4	pushl %esi; L(PUSHSI2): _PUSHCARGS_3
# define _POPCARGS_4	_POPCARGS_3; popl %esi; L(POPSI2):

# define PUSHCARGS_5	_PUSHCARGS_5
# define DOCARGS_5	_DOARGS_5 (36)
# define POPCARGS_5	_POPCARGS_5
# define _PUSHCARGS_5	pushl %edi; L(PUSHDI2): _PUSHCARGS_4
# define _POPCARGS_5	_POPCARGS_4; popl %edi; L(POPDI2):

# ifdef IS_IN_libpthread
#  define CENABLE	call __pthread_enable_asynccancel;
#  define CDISABLE	call __pthread_disable_asynccancel
# elif !defined NOT_IN_libc
#  define CENABLE	call __libc_enable_asynccancel;
#  define CDISABLE	call __libc_disable_asynccancel
# elif defined IS_IN_librt
#  define CENABLE	call __librt_enable_asynccancel;
#  define CDISABLE	call __librt_disable_asynccancel
# else
#  error Unsupported library
# endif
# define POPSTATE_0 \
 pushl %eax; L(PUSHSTATE): movl %ecx, %eax; CDISABLE; popl %eax; L(POPSTATE):
# define POPSTATE_1	POPSTATE_0
# define POPSTATE_2	xchgl (%esp), %eax; CDISABLE; popl %eax; L(POPSTATE):
# define POPSTATE_3	POPSTATE_2
# define POPSTATE_4	POPSTATE_3
# define POPSTATE_5	POPSTATE_4

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P cmpl $0, %gs:MULTIPLE_THREADS_OFFSET
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif
