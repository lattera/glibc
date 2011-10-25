#define __docos __docos_fma4
#define __dubcos __dubcos_fma4
#define __dubsin __dubsin_fma4
#define SECTION __attribute__ ((section (".text.fma4")))

#include <sysdeps/ieee754/dbl-64/dosincos.c>
