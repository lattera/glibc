#define __mpsqrt __mpsqrt_avx
#define __dbl_mp __dbl_mp_avx
#define __mul __mul_avx
#define __sub __sub_avx
#define AVOID_MPSQRT_H 1
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/mpsqrt.c>
