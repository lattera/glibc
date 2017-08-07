#define __mpexp __mpexp_fma
#define __add __add_fma
#define __dbl_mp __dbl_mp_fma
#define __dvd __dvd_fma
#define __mul __mul_fma
#define AVOID_MPEXP_H 1
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mpexp.c>
