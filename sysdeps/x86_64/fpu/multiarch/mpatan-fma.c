#define __mpatan __mpatan_fma
#define __add __add_fma
#define __dvd __dvd_fma
#define __mpsqrt __mpsqrt_fma
#define __mul __mul_fma
#define __sub __sub_fma
#define AVOID_MPATAN_H 1
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mpatan.c>
