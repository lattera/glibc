#define __mpatan2 __mpatan2_avx
#define __add __add_avx
#define __dvd __dvd_avx
#define __mpatan __mpatan_avx
#define __mpsqrt __mpsqrt_avx
#define __mul __mul_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mpatan2.c>
