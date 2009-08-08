/* Test case for x86-64 preserved registers in dynamic linker.  */

#include <stdlib.h>
#include <string.h>
#include <cpuid.h>
#include <emmintrin.h>

extern __m128i audit_test (__m128i, __m128i, __m128i, __m128i,
			   __m128i, __m128i, __m128i, __m128i);

int
main (void)
{
  unsigned int eax, ebx, ecx, edx;

  /* Run AVX test only if AVX is supported.  */
  if (__get_cpuid (1, &eax, &ebx, &ecx, &edx)
      && (ecx & bit_AVX))
    {
      __m128i xmm = _mm_setzero_si128 ();
      __m128i ret = audit_test (xmm, xmm, xmm, xmm, xmm, xmm, xmm, xmm);

      xmm = _mm_set1_epi32 (0x98abcdef);
      if (memcmp (&xmm, &ret, sizeof (ret)))
	abort ();
    }
  return 0;
}
