#define _FP_W_TYPE_SIZE		64
#define _FP_W_TYPE		unsigned long
#define _FP_WS_TYPE		signed long
#define _FP_I_TYPE		long

#define __FP_CLZ(r, x)							\
  do {									\
    __asm__("bsrq %1,%0" : "=r"(r) : "g"(x) : "cc");			\
    r ^= 63;								\
  } while (0)

#define _FP_NANFRAC_S		_FP_QNANBIT_S
#define _FP_NANFRAC_D		_FP_QNANBIT_D, 0
#define _FP_NANFRAC_Q		_FP_QNANBIT_Q, 0, 0, 0
#define _FP_NANSIGN_S		1
#define _FP_NANSIGN_D		1
#define _FP_NANSIGN_Q		1

#define _FP_KEEPNANFRACP 1
/* Here is something Intel misdesigned: the specs don't define
   the case where we have two NaNs with same mantissas, but
   different sign. Different operations pick up different NaNs.
 */
#define _FP_CHOOSENAN(fs, wc, R, X, Y, OP)			\
  do {								\
    if (_FP_FRAC_GT_##wc(X, Y)					\
	|| (_FP_FRAC_EQ_##wc(X,Y) && (OP == '+' || OP == '*')))	\
      {								\
	R##_s = X##_s;						\
        _FP_FRAC_COPY_##wc(R,X);				\
      }								\
    else							\
      {								\
	R##_s = Y##_s;						\
        _FP_FRAC_COPY_##wc(R,Y);				\
      }								\
    R##_c = FP_CLS_NAN;						\
  } while (0)

#define FP_EX_INVALID           (1 << 0)
#define FP_EX_DENORM		(1 << 1)
#define FP_EX_DIVZERO           (1 << 2)
#define FP_EX_OVERFLOW          (1 << 3)
#define FP_EX_UNDERFLOW         (1 << 4)
#define FP_EX_INEXACT           (1 << 5)

#define FP_RND_NEAREST		0
#define FP_RND_ZERO		3
#define FP_RND_PINF		2
#define FP_RND_MINF		1

