#define __docos __docos_avx
#define __dubcos __dubcos_avx
#define __dubsin __dubsin_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/dosincos.c>
