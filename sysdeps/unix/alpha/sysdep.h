/* Copyright (C) 1992, 1995, 1996, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sysdeps/unix/sysdep.h>

#ifdef __ASSEMBLER__

#ifdef __linux__
# include <alpha/regdef.h>
#else
# include <regdef.h>
#endif

#ifdef __STDC__
#define __LABEL(x)	x##:
#else
#define __LABEL(x)	x/**/:
#endif

#define LEAF(name, framesize)			\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, framesize, ra

#define ENTRY(name)				\
  .globl name;					\
  .align 3;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, 0, ra

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.end sym

/* Note that PSEUDO/PSEUDO_END use label number 1996---do not use a
   label of that number between those two macros!  */

#ifdef PROF
#define PSEUDO(name, syscall_name, args)	\
    .globl name;				\
    .align 3;					\
    .ent name,0;				\
__LABEL(name)					\
    .frame sp, 0, ra;				\
    ldgp gp,0(pv);				\
    .set noat;					\
    lda AT,_mcount;				\
    jsr AT,(AT),_mcount;			\
    .set at;					\
    .prologue 1;				\
    ldiq	v0, SYS_ify(syscall_name);	\
    .set noat;					\
    call_pal	PAL_callsys;			\
    .set at;					\
    bne		a3, 1996f;			\
3:
#else
#define PSEUDO(name, syscall_name, args)	\
    .globl name;				\
    .align 3;					\
    .ent name,0;				\
__LABEL(name)					\
    .frame sp, 0, ra				\
    .prologue 0;				\
    ldiq	v0, SYS_ify(syscall_name);	\
    .set noat;					\
    call_pal	PAL_callsys;			\
    .set at;					\
    bne		a3, 1996f;			\
3:
#endif

#undef PSEUDO_END
#ifdef PROF
#define PSEUDO_END(sym)				\
1996:						\
    jmp		zero, __syscall_error;		\
    END(sym)
#else
#define PSEUDO_END(sym)				\
1996:						\
    br		gp, 2f;				\
2:  ldgp	gp, 0(gp);			\
    jmp		zero, __syscall_error;		\
    END(sym)
#endif

#define r0	v0
#define r1	a4

#define MOVE(x,y)	mov x,y

#else /* !ASSEMBLER */

/* Define a macro which expands inline into the wrapper code for a
   system call.  */

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)  INLINE_SYSCALL1(name, nr, args)

#define INLINE_SYSCALL1(name, nr, args...)	\
({						\
	long _sc_ret, _sc_err;			\
	inline_syscall##nr(name, args);		\
	if (_sc_err)				\
	  {					\
	    __set_errno (_sc_ret);		\
	    _sc_ret = -1L;			\
	  }					\
	_sc_ret;				\
})

#define inline_syscall_clobbers				\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"

/* It is moderately important optimization-wise to limit the lifetime
   of the hard-register variables as much as possible.  Thus we copy
   in/out as close to the asm as possible.  */

#define inline_syscall0(name)			\
{						\
	register long _sc_0 __asm__("$0");	\
	register long _sc_19 __asm__("$19");	\
						\
	_sc_0 = __NR_##name;			\
	__asm__("callsys # %0 %1 <= %2"		\
		: "=r"(_sc_0), "=r"(_sc_19)	\
		: "0"(_sc_0)			\
		: inline_syscall_clobbers);	\
	_sc_ret = _sc_0, _sc_err = _sc_19;	\
}

#define inline_syscall1(name,arg1)		\
{						\
	register long _sc_0 __asm__("$0");	\
	register long _sc_16 __asm__("$16");	\
	register long _sc_19 __asm__("$19");	\
						\
	_sc_0 = __NR_##name;			\
	_sc_16 = (long) (arg1);			\
	__asm__("callsys # %0 %1 <= %2 %3"	\
		: "=r"(_sc_0), "=r"(_sc_19)	\
		: "0"(_sc_0), "r"(_sc_16)	\
		: inline_syscall_clobbers);	\
	_sc_ret = _sc_0, _sc_err = _sc_19;	\
}

#define inline_syscall2(name,arg1,arg2)			\
{							\
	register long _sc_0 __asm__("$0");		\
	register long _sc_16 __asm__("$16");		\
	register long _sc_17 __asm__("$17");		\
	register long _sc_19 __asm__("$19");		\
							\
	_sc_0 = __NR_##name;				\
	_sc_16 = (long) (arg1);				\
	_sc_17 = (long) (arg2);				\
	__asm__("callsys # %0 %1 <= %2 %3 %4"		\
		: "=r"(_sc_0), "=r"(_sc_19)		\
		: "0"(_sc_0), "r"(_sc_16), "r"(_sc_17)	\
		: inline_syscall_clobbers);		\
	_sc_ret = _sc_0, _sc_err = _sc_19;		\
}

#define inline_syscall3(name,arg1,arg2,arg3)		\
{							\
	register long _sc_0 __asm__("$0");		\
	register long _sc_16 __asm__("$16");		\
	register long _sc_17 __asm__("$17");		\
	register long _sc_18 __asm__("$18");		\
	register long _sc_19 __asm__("$19");		\
							\
	_sc_0 = __NR_##name;				\
	_sc_16 = (long) (arg1);				\
	_sc_17 = (long) (arg2);				\
	_sc_18 = (long) (arg3);				\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5"	\
		: "=r"(_sc_0), "=r"(_sc_19)		\
		: "0"(_sc_0), "r"(_sc_16), "r"(_sc_17),	\
		  "r"(_sc_18)				\
		: inline_syscall_clobbers);		\
	_sc_ret = _sc_0, _sc_err = _sc_19;		\
}

#define inline_syscall4(name,arg1,arg2,arg3,arg4)	\
{							\
	register long _sc_0 __asm__("$0");		\
	register long _sc_16 __asm__("$16");		\
	register long _sc_17 __asm__("$17");		\
	register long _sc_18 __asm__("$18");		\
	register long _sc_19 __asm__("$19");		\
							\
	_sc_0 = __NR_##name;				\
	_sc_16 = (long) (arg1);				\
	_sc_17 = (long) (arg2);				\
	_sc_18 = (long) (arg3);				\
	_sc_19 = (long) (arg4);				\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6"	\
		: "=r"(_sc_0), "=r"(_sc_19)		\
		: "0"(_sc_0), "r"(_sc_16), "r"(_sc_17),	\
		  "r"(_sc_18), "1"(_sc_19)		\
		: inline_syscall_clobbers);		\
	_sc_ret = _sc_0, _sc_err = _sc_19;		\
}

#define inline_syscall5(name,arg1,arg2,arg3,arg4,arg5)	\
{							\
	register long _sc_0 __asm__("$0");		\
	register long _sc_16 __asm__("$16");		\
	register long _sc_17 __asm__("$17");		\
	register long _sc_18 __asm__("$18");		\
	register long _sc_19 __asm__("$19");		\
	register long _sc_20 __asm__("$20");		\
							\
	_sc_0 = __NR_##name;				\
	_sc_16 = (long) (arg1);				\
	_sc_17 = (long) (arg2);				\
	_sc_18 = (long) (arg3);				\
	_sc_19 = (long) (arg4);				\
	_sc_20 = (long) (arg5);				\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7"	\
		: "=r"(_sc_0), "=r"(_sc_19)		\
		: "0"(_sc_0), "r"(_sc_16), "r"(_sc_17),	\
		  "r"(_sc_18), "1"(_sc_19), "r"(_sc_20)	\
		: inline_syscall_clobbers);		\
	_sc_ret = _sc_0, _sc_err = _sc_19;		\
}

#define inline_syscall6(name,arg1,arg2,arg3,arg4,arg5,arg6)	\
{								\
	register long _sc_0 __asm__("$0");			\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _sc_20 __asm__("$20");			\
	register long _sc_21 __asm__("$21");			\
								\
	_sc_0 = __NR_##name;					\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	_sc_19 = (long) (arg4);					\
	_sc_20 = (long) (arg5);					\
	_sc_21 = (long) (arg6);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7 %8"	\
		: "=r"(_sc_0), "=r"(_sc_19)			\
		: "0"(_sc_0), "r"(_sc_16), "r"(_sc_17),		\
		  "r"(_sc_18), "1"(_sc_19), "r"(_sc_20),	\
		  "r"(_sc_21)					\
		: inline_syscall_clobbers);			\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#endif /* ASSEMBLER */
