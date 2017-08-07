#define __add __add_fma
#define __mul __mul_fma
#define __sqr __sqr_fma
#define __sub __sub_fma
#define __dbl_mp __dbl_mp_fma
#define __dvd __dvd_fma

#define NO___CPY 1
#define NO___MP_DBL 1
#define NO___ACR 1
#define NO__CONST 1
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/mpa.c>
