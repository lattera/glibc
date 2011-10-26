#define __cos32 __cos32_avx
#define __sin32 __sin32_avx
#define __c32 __c32_avx
#define __mpsin __mpsin_avx
#define __mpsin1 __mpsin1_avx
#define __mpcos __mpcos_avx
#define __mpcos1 __mpcos1_avx
#define __mpranred __mpranred_avx
#define __add __add_avx
#define __dbl_mp __dbl_mp_avx
#define __mul __mul_avx
#define __sub __sub_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/sincos32.c>
