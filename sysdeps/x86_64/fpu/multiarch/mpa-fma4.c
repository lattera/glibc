#define __add __add_fma4
#define __mul __mul_fma4
#define __sqr __sqr_fma4
#define __sub __sub_fma4
#define __dbl_mp __dbl_mp_fma4
#define __dvd __dvd_fma4

#define NO___CPY 1
#define NO___MP_DBL 1
#define NO___ACR 1
#define NO__CONST 1
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/mpa.c>
