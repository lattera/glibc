#if IS_IN (libc)
# define VEC_SIZE	32
# define VEC(i)		ymm##i
# define VMOVU		vmovdqu
# define VMOVA		vmovdqa

# define VDUP_TO_VEC0_AND_SET_RETURN(d, r) \
  vmovd d, %xmm0; \
  movq r, %rax; \
  vpbroadcastb %xmm0, %ymm0

# define SECTION(p)		p##.avx
# define MEMSET_SYMBOL(p,s)	p##_avx2_##s

# include "memset-vec-unaligned-erms.S"
#endif
