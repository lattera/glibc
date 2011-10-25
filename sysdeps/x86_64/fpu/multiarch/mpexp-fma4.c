#define __mpexp __mpexp_fma4
#define __add __add_fma4
#define __dbl_mp __dbl_mp_fma4
#define __dvd __dvd_fma4
#define __mul __mul_fma4
#define AVOID_MPEXP_H 1
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mpexp.c>
