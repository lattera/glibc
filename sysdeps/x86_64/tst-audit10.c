/* Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

/* Test case for x86-64 preserved registers in dynamic linker.  */

#ifdef __AVX512F__
#include <stdlib.h>
#include <string.h>
#include <cpuid.h>
#include <immintrin.h>

static int
avx512_enabled (void)
{
  unsigned int eax, ebx, ecx, edx;

  if (__get_cpuid (1, &eax, &ebx, &ecx, &edx) == 0
      || (ecx & (bit_AVX | bit_OSXSAVE)) != (bit_AVX | bit_OSXSAVE))
    return 0;

  __cpuid_count (7, 0, eax, ebx, ecx, edx);
  if (!(ebx & bit_AVX512F))
    return 0;

  asm ("xgetbv" : "=a" (eax), "=d" (edx) : "c" (0));

  /* Verify that ZMM, YMM and XMM states are enabled.  */
  return (eax & 0xe6) == 0xe6;
}


extern __m512i audit_test (__m512i, __m512i, __m512i, __m512i,
			   __m512i, __m512i, __m512i, __m512i);
static int
do_test (void)
{
  /* Run AVX512 test only if AVX512 is supported.  */
  if (avx512_enabled ())
    {
      __m512i zmm = _mm512_setzero_si512 ();
      __m512i ret = audit_test (zmm, zmm, zmm, zmm, zmm, zmm, zmm, zmm);

      zmm = _mm512_set1_epi64 (0x12349876);

      if (memcmp (&zmm, &ret, sizeof (ret)))
	abort ();
    }
  return 0;
}
#else
static int
do_test (void)
{
  return 0;
}
#endif

#define TEST_FUNCTION do_test ()
#include "../../test-skeleton.c"
