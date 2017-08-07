#define __halfulp __halfulp_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/halfulp.c>
