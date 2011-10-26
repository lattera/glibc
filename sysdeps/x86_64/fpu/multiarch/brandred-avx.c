#define __branred __branred_avx
#define SECTION __attribute__ ((section (".text.avx")))

#include <sysdeps/ieee754/dbl-64/branred.c>
