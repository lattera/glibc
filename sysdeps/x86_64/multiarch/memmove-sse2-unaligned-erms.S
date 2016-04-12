#if IS_IN (libc)
# define VEC_SIZE	16
# define VEC(i)		xmm##i
# define VMOVNT		movntdq
/* Use movups and movaps for smaller code sizes.  */
# define VMOVU		movups
# define VMOVA		movaps

# define SECTION(p)		p
# define MEMMOVE_SYMBOL(p,s)	p##_sse2_##s

# include "memmove-vec-unaligned-erms.S"
#endif
