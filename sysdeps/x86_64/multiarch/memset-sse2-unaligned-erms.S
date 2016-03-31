#define VEC_SIZE	16
#define VEC(i)		xmm##i
#define VMOVU		movdqu
#define VMOVA		movdqa

#define VDUP_TO_VEC0_AND_SET_RETURN(d, r) \
  movd d, %xmm0; \
  movq r, %rax; \
  punpcklbw %xmm0, %xmm0; \
  punpcklwd %xmm0, %xmm0; \
  pshufd $0, %xmm0, %xmm0

#define SECTION(p)		p
#define MEMSET_SYMBOL(p,s)	p##_sse2_##s

#include "memset-vec-unaligned-erms.S"
