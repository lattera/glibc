#define VEC_SIZE	16
#define VEC(i)		xmm##i
#define VMOVU		movdqu
#define VMOVA		movdqa

#define SECTION(p)		p
#define MEMMOVE_SYMBOL(p,s)	p##_sse2_##s

#include "memmove-vec-unaligned-erms.S"
