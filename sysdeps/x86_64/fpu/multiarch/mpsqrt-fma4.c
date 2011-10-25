#define __mpsqrt __mpsqrt_fma4
#define __dbl_mp __dbl_mp_fma4
#define __mul __mul_fma4
#define __sub __sub_fma4
#define AVOID_MPSQRT_H 1
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mpsqrt.c>
