#define __mpsqrt __mpsqrt_fma
#define __dbl_mp __dbl_mp_fma
#define __mul __mul_fma
#define __sub __sub_fma
#define AVOID_MPSQRT_H 1
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mpsqrt.c>
