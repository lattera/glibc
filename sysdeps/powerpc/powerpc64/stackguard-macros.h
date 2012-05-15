#include <stdint.h>

#define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ld %0,-28688(13)" : "=r" (x)); x; })
