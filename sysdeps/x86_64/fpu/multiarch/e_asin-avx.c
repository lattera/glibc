#define __ieee754_acos __ieee754_acos_avx
#define __ieee754_asin __ieee754_asin_avx
#define __cos32 __cos32_avx
#define __doasin __doasin_avx
#define __docos __docos_avx
#define __dubcos __dubcos_avx
#define __dubsin __dubsin_avx
#define __sin32 __sin32_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/e_asin.c>
