/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tls.h>
#ifndef __ASSEMBLER__
# include <nptl/pthreadP.h>
#endif

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# undef PSEUDO

# if IS_IN (libc)
#  define SYSDEP_CANCEL_ERRNO __libc_errno
# else
#  define SYSDEP_CANCEL_ERRNO errno
# endif
# define SYSDEP_CANCEL_ERROR(args)					      \
.section .gnu.linkonce.t.__syscall_error_##args, "ax";			      \
     .align 32;								      \
     .proc __syscall_error_##args;					      \
     .global __syscall_error_##args;					      \
     .hidden __syscall_error_##args;					      \
     .size __syscall_error_##args, 64;					      \
__syscall_error_##args:							      \
     .prologue;								      \
     .regstk args, 5, args, 0;						      \
     .save ar.pfs, loc0;						      \
     .save rp, loc1;							      \
     .body;								      \
     addl loc4 = @ltoff(@tprel(SYSDEP_CANCEL_ERRNO)), gp;;		      \
     ld8 loc4 = [loc4];							      \
     mov rp = loc1;;							      \
     mov r8 = -1;							      \
     add loc4 = loc4, r13;;						      \
     st4 [loc4] = loc3;							      \
     mov ar.pfs = loc0

# ifndef USE_DL_SYSINFO

#  define PSEUDO(name, syscall_name, args)				      \
.text;									      \
ENTRY (name)								      \
     adds r14 = MULTIPLE_THREADS_OFFSET, r13;;				      \
     ld4 r14 = [r14];							      \
     mov r15 = SYS_ify(syscall_name);;					      \
     cmp4.ne p6, p7 = 0, r14;						      \
(p6) br.cond.spnt .Lpseudo_cancel;;					      \
     break __BREAK_SYSCALL;;						      \
     cmp.eq p6,p0=-1,r10;						      \
(p6) br.cond.spnt.few __syscall_error;					      \
     ret;;								      \
     .endp name;							      \
     .proc __GC_##name;							      \
     .globl __GC_##name;						      \
     .hidden __GC_##name;						      \
__GC_##name:								      \
.Lpseudo_cancel:							      \
     .prologue;								      \
     .regstk args, 5, args, 0;						      \
     .save ar.pfs, loc0;						      \
     alloc loc0 = ar.pfs, args, 5, args, 0;				      \
     .save rp, loc1;							      \
     mov loc1 = rp;;							      \
     .body;								      \
     CENABLE;;								      \
     mov loc2 = r8;							      \
     COPY_ARGS_##args							      \
     mov r15 = SYS_ify(syscall_name);					      \
     break __BREAK_SYSCALL;;						      \
     mov loc3 = r8;							      \
     mov loc4 = r10;							      \
     mov out0 = loc2;							      \
     CDISABLE;;								      \
     cmp.eq p6,p0=-1,loc4;						      \
(p6) br.cond.spnt.few __syscall_error_##args;				      \
     mov r8 = loc3;							      \
     mov rp = loc1;							      \
     mov ar.pfs = loc0;							      \
.Lpseudo_end:								      \
     ret;								      \
     .endp __GC_##name;							      \
     SYSDEP_CANCEL_ERROR(args)

# else /* USE_DL_SYSINFO */

#  define PSEUDO(name, syscall_name, args)				      \
.text;									      \
ENTRY (name)								      \
     .prologue;								      \
     adds r2 = SYSINFO_OFFSET, r13;					      \
     adds r14 = MULTIPLE_THREADS_OFFSET, r13;				      \
     .save ar.pfs, r11;							      \
     mov r11 = ar.pfs;;							      \
     .body;								      \
     ld4 r14 = [r14];							      \
     ld8 r2 = [r2];							      \
     mov r15 = SYS_ify(syscall_name);;					      \
     cmp4.ne p6, p7 = 0, r14;						      \
     mov b7 = r2;							      \
(p6) br.cond.spnt .Lpseudo_cancel;					      \
     br.call.sptk.many b6 = b7;;					      \
     mov ar.pfs = r11;							      \
     cmp.eq p6,p0 = -1, r10;						      \
(p6) br.cond.spnt.few __syscall_error;					      \
     ret;;								      \
     .endp name;							      \
									      \
      .proc __##syscall_name##_nocancel;				      \
     .globl __##syscall_name##_nocancel;				      \
__##syscall_name##_nocancel:						      \
     .prologue;								      \
     adds r2 = SYSINFO_OFFSET, r13;					      \
     .save ar.pfs, r11;							      \
     mov r11 = ar.pfs;;							      \
     .body;								      \
     ld8 r2 = [r2];							      \
     mov r15 = SYS_ify(syscall_name);;					      \
     mov b7 = r2;							      \
     br.call.sptk.many b6 = b7;;					      \
     mov ar.pfs = r11;							      \
     cmp.eq p6,p0 = -1, r10;						      \
(p6) br.cond.spnt.few __syscall_error;					      \
     ret;;								      \
     .endp __##syscall_name##_nocancel;					      \
									      \
     .proc __GC_##name;							      \
     .globl __GC_##name;						      \
     .hidden __GC_##name;						      \
__GC_##name:								      \
.Lpseudo_cancel:							      \
     .prologue;								      \
     .regstk args, 5, args, 0;						      \
     .save ar.pfs, loc0;						      \
     alloc loc0 = ar.pfs, args, 5, args, 0;				      \
     adds loc4 = SYSINFO_OFFSET, r13;					      \
     .save rp, loc1;							      \
     mov loc1 = rp;;							      \
     .body;								      \
     ld8 loc4 = [loc4];							      \
     CENABLE;;								      \
     mov loc2 = r8;							      \
     mov b7 = loc4;							      \
     COPY_ARGS_##args							      \
     mov r15 = SYS_ify(syscall_name);					      \
     br.call.sptk.many b6 = b7;;					      \
     mov loc3 = r8;							      \
     mov loc4 = r10;							      \
     mov out0 = loc2;							      \
     CDISABLE;;								      \
     cmp.eq p6,p0=-1,loc4;						      \
(p6) br.cond.spnt.few __syscall_error_##args;				      \
     mov r8 = loc3;							      \
     mov rp = loc1;							      \
     mov ar.pfs = loc0;							      \
.Lpseudo_end:								      \
     ret;								      \
     .endp __GC_##name;							      \
     SYSDEP_CANCEL_ERROR(args)

# endif /* USE_DL_SYSINFO */

# undef PSEUDO_END
# define PSEUDO_END(name) .endp

# if IS_IN (libpthread)
#  define CENABLE	br.call.sptk.many b0 = __pthread_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __pthread_disable_asynccancel
# elif IS_IN (libc)
#  define CENABLE	br.call.sptk.many b0 = __libc_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __libc_disable_asynccancel
# elif IS_IN (librt)
#  define CENABLE	br.call.sptk.many b0 = __librt_enable_asynccancel
#  define CDISABLE	br.call.sptk.many b0 = __librt_disable_asynccancel
# else
#  error Unsupported library
# endif

# define COPY_ARGS_0	/* Nothing */
# define COPY_ARGS_1	COPY_ARGS_0 mov out0 = in0;
# define COPY_ARGS_2	COPY_ARGS_1 mov out1 = in1;
# define COPY_ARGS_3	COPY_ARGS_2 mov out2 = in2;
# define COPY_ARGS_4	COPY_ARGS_3 mov out3 = in3;
# define COPY_ARGS_5	COPY_ARGS_4 mov out4 = in4;
# define COPY_ARGS_6	COPY_ARGS_5 mov out5 = in5;
# define COPY_ARGS_7	COPY_ARGS_6 mov out6 = in6;

# ifndef __ASSEMBLER__
#  define SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, header.multiple_threads) == 0, 1)
# else
#  define SINGLE_THREAD_P \
  adds r14 = MULTIPLE_THREADS_OFFSET, r13 ;; ld4 r14 = [r14] ;; cmp4.ne p6, p7 = 0, r14
# endif

#elif !defined __ASSEMBLER__

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif

#ifndef __ASSEMBLER__
# define RTLD_SINGLE_THREAD_P \
  __builtin_expect (THREAD_GETMEM (THREAD_SELF, \
				   header.multiple_threads) == 0, 1)
#endif
