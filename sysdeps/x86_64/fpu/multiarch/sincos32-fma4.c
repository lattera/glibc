#define __cos32 __cos32_fma4
#define __sin32 __sin32_fma4
#define __c32 __c32_fma4
#define __mpsin __mpsin_fma4
#define __mpsin1 __mpsin1_fma4
#define __mpcos __mpcos_fma4
#define __mpcos1 __mpcos1_fma4
#define __mpranred __mpranred_fma4
#define __add __add_fma4
#define __dbl_mp __dbl_mp_fma4
#define __mul __mul_fma4
#define __sub __sub_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/sincos32.c>
