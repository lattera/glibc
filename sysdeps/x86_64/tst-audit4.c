/* Test case for x86-64 preserved registers in dynamic linker.  */

#ifdef __AVX__
#include <stdlib.h>
#include <string.h>
#include <cpuid.h>
#include <immintrin.h>


static int
avx_enabled (void)
{
  unsigned int eax, ebx, ecx, edx;

  if (__get_cpuid (1, &eax, &ebx, &ecx, &edx) == 0
      || (ecx & (bit_AVX | bit_OSXSAVE)) != (bit_AVX | bit_OSXSAVE))
    return 0;

  /* Check the OS has AVX and SSE saving enabled.  */
  asm ("xgetbv" : "=a" (eax), "=d" (edx) : "c" (0));

  return (eax & 6) == 6;
}


extern __m256i audit_test (__m256i, __m256i, __m256i, __m256i,
			   __m256i, __m256i, __m256i, __m256i);
int
main (void)
{
  /* Run AVX test only if AVX is supported.  */
  if (avx_enabled ())
    {
      __m256i ymm = _mm256_setzero_si256 ();
      __m256i ret = audit_test (ymm, ymm, ymm, ymm, ymm, ymm, ymm, ymm);

      ymm =  _mm256_set1_epi32 (0x12349876);
      if (memcmp (&ymm, &ret, sizeof (ret)))
	abort ();
    }
  return 0;
}
#else
int
main (void)
{
  return 0;
}
#endif
