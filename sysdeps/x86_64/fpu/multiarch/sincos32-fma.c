#define __cos32 __cos32_fma
#define __sin32 __sin32_fma
#define __c32 __c32_fma
#define __mpsin __mpsin_fma
#define __mpsin1 __mpsin1_fma
#define __mpcos __mpcos_fma
#define __mpcos1 __mpcos1_fma
#define __mpranred __mpranred_fma
#define __add __add_fma
#define __dbl_mp __dbl_mp_fma
#define __mul __mul_fma
#define __sub __sub_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/sincos32.c>
