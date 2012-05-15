#include <stdint.h>

#define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ld [%%g7+0x14], %0" : "=r" (x)); x; })
