#include <stdint.h>

#ifdef __i386__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("movl %%gs:0x14, %0" : "=r" (x)); x; })
#elif defined __x86_64__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("movq %%fs:0x28, %0" : "=r" (x)); x; })
#elif defined __powerpc64__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ld %0,-28688(13)" : "=r" (x)); x; })
#elif defined __powerpc__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("lwz %0,-28680(2)" : "=r" (x)); x; })
#elif defined __sparc__ && defined __arch64__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ldx [%%g7+0x28], %0" : "=r" (x)); x; })
#elif defined __sparc__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ld [%%g7+0x14], %0" : "=r" (x)); x; })
#elif defined __s390x__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ear %0,%%a0; sllg %0,%0,32; ear %0,%%a1; lg %0,0x28(%0)" : "=a" (x)); x; })
#elif defined __s390__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("ear %0,%%a0; l %0,0x14(%0)" : "=a" (x)); x; })
#elif defined __ia64__
# define STACK_CHK_GUARD \
  ({ uintptr_t x; asm ("adds %0 = -8, r13;; ld8 %0 = [%0]" : "=r" (x)); x; })
#else
extern uintptr_t __stack_chk_guard;
# define STACK_CHK_GUARD __stack_chk_guard
#endif
