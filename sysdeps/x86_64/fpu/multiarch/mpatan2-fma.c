#define __mpatan2 __mpatan2_fma
#define __add __add_fma
#define __dvd __dvd_fma
#define __mpatan __mpatan_fma
#define __mpsqrt __mpsqrt_fma
#define __mul __mul_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mpatan2.c>
