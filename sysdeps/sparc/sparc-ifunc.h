/* This file is part of the GNU C Library.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.

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

#ifdef __ASSEMBLER__

# ifdef SHARED

#  define SPARC_ASM_IFUNC_DFLT(name, dflt)		\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	SETUP_PIC_REG_LEAF(o3, o5);			\
	sethi	%gdop_hix22(dflt), %o1;			\
	xor	%o1, %gdop_lox10(dflt), %o1;		\
	add	%o3, %o1, %o1;				\
	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

#  define SPARC_ASM_IFUNC1(name, m1, f1, dflt)		\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	SETUP_PIC_REG_LEAF(o3, o5);			\
	set	m1, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	9f;					\
	 nop;						\
	sethi	%gdop_hix22(f1), %o1;			\
	xor	%o1, %gdop_lox10(f1), %o1;		\
	ba	10f;					\
	 nop;						\
9:	sethi	%gdop_hix22(dflt), %o1;			\
	xor	%o1, %gdop_lox10(dflt), %o1;		\
10:	add	%o3, %o1, %o1;				\
	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

#  define SPARC_ASM_IFUNC2(name, m1, f1, m2, f2, dflt)	\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	SETUP_PIC_REG_LEAF(o3, o5);			\
	set	m1, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	8f;					\
	 nop;						\
	sethi	%gdop_hix22(f1), %o1;			\
	xor	%o1, %gdop_lox10(f1), %o1;		\
	ba	10f;					\
	 nop;						\
8:	set	m2, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	9f;					\
	 nop;						\
	sethi	%gdop_hix22(f2), %o1;			\
	xor	%o1, %gdop_lox10(f2), %o1;		\
	ba	10f;					\
	 nop;						\
9:	sethi	%gdop_hix22(dflt), %o1;			\
	xor	%o1, %gdop_lox10(dflt), %o1;		\
10:	add	%o3, %o1, %o1;				\
	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

# else /* SHARED */

# ifdef __arch64__
#  define SET(SYM, TMP, REG)	setx SYM, TMP, REG
# else
#  define SET(SYM, TMP, REG)	set SYM, REG
# endif

#  define SPARC_ASM_IFUNC_DFLT(name, dflt)		\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	SET(dflt, %g1, %o1);				\
	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

#  define SPARC_ASM_IFUNC1(name, m1, f1, dflt)		\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	set	m1, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	9f;					\
	 nop;						\
	SET(f1, %g1, %o1);				\
	ba	10f;					\
	 nop;						\
9:	SET(dflt, %g1, %o1);				\
10:	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

#  define SPARC_ASM_IFUNC2(name, m1, f1, m2, f2, dflt)	\
ENTRY (__##name)					\
	.type	__##name, @gnu_indirect_function;	\
	set	m1, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	8f;					\
	 nop;						\
	SET(f1, %g1, %o1);				\
	ba	10f;					\
	 nop;						\
8:	set	m2, %o1;				\
	andcc	%o0, %o1, %g0;				\
	be	9f;					\
	 nop;						\
	SET(f2, %g1, %o1);				\
	ba	10f;					\
	 nop;						\
9:	SET(dflt, %g1, %o1);				\
10:	retl;						\
	 mov	%o1, %o0;				\
END (__##name)

# endif /* SHARED */

#define SPARC_ASM_VIS2_IFUNC(name)			\
	SPARC_ASM_IFUNC1(name, HWCAP_SPARC_VIS2,	\
			 __##name##_vis2, __##name##_generic)

# ifdef HAVE_AS_VIS3_SUPPORT

#define SPARC_ASM_VIS3_IFUNC(name)			\
	SPARC_ASM_IFUNC1(name, HWCAP_SPARC_VIS3,	\
			 __##name##_vis3, __##name##_generic)

#define SPARC_ASM_VIS3_VIS2_IFUNC(name)			\
	SPARC_ASM_IFUNC2(name, HWCAP_SPARC_VIS3,	\
			 __##name##_vis3,		\
			 HWCAP_SPARC_VIS2,		\
			 __##name##_vis2, __##name##_generic)

# else /* HAVE_AS_VIS3_SUPPORT */

#define SPARC_ASM_VIS3_IFUNC(name)			\
	SPARC_ASM_IFUNC_DFLT(name, __##name##_generic)

#define SPARC_ASM_VIS3_VIS2_IFUNC(name)			\
	SPARC_ASM_VIS2_IFUNC(name)

# endif /* HAVE_AS_VIS3_SUPPORT */


#else	/* __ASSEMBLER__ */

# define sparc_libm_ifunc(name, expr)					\
  extern void *name##_ifunc (int) __asm__ (#name);			\
  void *name##_ifunc (int hwcap)					\
  {									\
    __typeof (name) *res = expr;					\
    return res;								\
  }									\
  __asm__ (".type " #name ", %gnu_indirect_function");

# define sparc_libc_ifunc(name, expr) sparc_libm_ifunc (name, expr)

#endif	/* __ASSEMBLER__ */
