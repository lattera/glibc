#define __mpatan __mpatan_fma4
#define __add __add_fma4
#define __dvd __dvd_fma4
#define __mpsqrt __mpsqrt_fma4
#define __mul __mul_fma4
#define __sub __sub_fma4
#define AVOID_MPATAN_H 1
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mpatan.c>
