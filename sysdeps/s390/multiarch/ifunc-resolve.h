/* IFUNC resolver function for CPU specific functions.
   32/64 bit S/390 version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <dl-procinfo.h>

#define S390_STFLE_BITS_Z10  34 /* General instructions extension */
#define S390_STFLE_BITS_Z196 45 /* Distinct operands, pop ... */

#define S390_IS_Z196(STFLE_BITS)			\
  ((STFLE_BITS & (1ULL << (63 - S390_STFLE_BITS_Z196))) != 0)

#define S390_IS_Z10(STFLE_BITS)				\
  ((STFLE_BITS & (1ULL << (63 - S390_STFLE_BITS_Z10))) != 0)

#define S390_STORE_STFLE(STFLE_BITS)					\
  /* We want just 1 double word to be returned.  */			\
  register unsigned long reg0 __asm__("0") = 0;				\
									\
  __asm__ __volatile__(".machine push"        "\n\t"			\
		       ".machine \"z9-109\""  "\n\t"			\
		       ".machinemode \"zarch_nohighgprs\"\n\t"		\
		       "stfle %0"             "\n\t"			\
		       ".machine pop"         "\n"			\
		       : "=QS" (STFLE_BITS), "+d" (reg0)		\
		       : : "cc");
#define s390_libc_ifunc_init()						\
  unsigned long long stfle_bits = 0ULL;					\
  if (__glibc_likely((dl_hwcap & HWCAP_S390_STFLE)			\
		     && (dl_hwcap & HWCAP_S390_ZARCH)			\
		     && (dl_hwcap & HWCAP_S390_HIGH_GPRS)))		\
    {									\
      S390_STORE_STFLE (stfle_bits);					\
    }

#define s390_libc_ifunc(TYPE_FUNC, RESOLVERFUNC, FUNC)			\
  /* Make the declarations of the optimized functions hidden in order
     to prevent GOT slots being generated for them. */			\
  extern __typeof (TYPE_FUNC) RESOLVERFUNC##_z196 attribute_hidden;	\
  extern __typeof (TYPE_FUNC) RESOLVERFUNC##_z10 attribute_hidden;      \
  extern __typeof (TYPE_FUNC) RESOLVERFUNC##_default attribute_hidden;  \
  __ifunc (TYPE_FUNC, FUNC,						\
	   __glibc_likely (S390_IS_Z196 (stfle_bits))			\
	   ? RESOLVERFUNC##_z196					\
	   : __glibc_likely (S390_IS_Z10 (stfle_bits))			\
	     ? RESOLVERFUNC##_z10					\
	     : RESOLVERFUNC##_default,					\
	   unsigned long int dl_hwcap, s390_libc_ifunc_init);

#define s390_vx_libc_ifunc(FUNC)		\
  s390_vx_libc_ifunc2_redirected(FUNC, FUNC, FUNC)

#define s390_vx_libc_ifunc_redirected(TYPE_FUNC, FUNC)	\
  s390_vx_libc_ifunc2_redirected(TYPE_FUNC, FUNC, FUNC)

#define s390_vx_libc_ifunc2(RESOLVERFUNC, FUNC)	\
  s390_vx_libc_ifunc2_redirected(FUNC, RESOLVERFUNC, FUNC)

#define s390_vx_libc_ifunc_init()
#define s390_vx_libc_ifunc2_redirected(TYPE_FUNC, RESOLVERFUNC, FUNC)	\
  /* Make the declarations of the optimized functions hidden in order
     to prevent GOT slots being generated for them.  */			\
  extern __typeof (TYPE_FUNC) RESOLVERFUNC##_vx attribute_hidden;	\
  extern __typeof (TYPE_FUNC) RESOLVERFUNC##_c attribute_hidden;	\
  __ifunc (TYPE_FUNC, FUNC,						\
	   (dl_hwcap & HWCAP_S390_VX)					\
	   ? RESOLVERFUNC##_vx						\
	   : RESOLVERFUNC##_c,						\
	   unsigned long int dl_hwcap, s390_vx_libc_ifunc_init);
