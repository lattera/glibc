#ifdef HAVE_AVX512_ASM_SUPPORT
# define VEC_SIZE	64
# define VEC(i)		zmm##i
# define VMOVU		vmovdqu64
# define VMOVA		vmovdqa64

# define VDUP_TO_VEC0_AND_SET_RETURN(d, r) \
  vmovd d, %xmm0; \
  movq r, %rax; \
  vpbroadcastb %xmm0, %xmm0; \
  vpbroadcastq %xmm0, %zmm0

# define SECTION(p)		p##.avx512
# define MEMSET_SYMBOL(p,s)	p##_avx512_##s

# include "memset-vec-unaligned-erms.S"
#endif
