#define __docos __docos_fma
#define __dubcos __dubcos_fma
#define __dubsin __dubsin_fma
#define SECTION __attribute__ ((section (".text.fma")))

#include <sysdeps/ieee754/dbl-64/dosincos.c>
