#define _FP_W_TYPE_SIZE		32
#define _FP_W_TYPE		unsigned long
#define _FP_WS_TYPE		signed long
#define _FP_I_TYPE		long

#define __FP_FRAC_ADD_2(rh, rl, xh, xl, yh, yl)				\
  __asm__("addl %5,%1; adcl %3,%0"					\
	  : "=r"(rh), "=r"(rl)						\
	  : "%0"(xh), "g"(yh), "%1"(xl), "g"(yl)			\
	  : "cc")

#define __FP_FRAC_ADD_4(r3,r2,r1,r0,x3,x2,x1,x0,y3,y2,y1,y0)		\
  do { 									\
    __asm__ volatile("addl %5,%1; adcl %3,%0"				\
		     : "=r"(r1), "=r"(r0)				\
		     : "%0"(x1), "g"(y1), "%1"(x0), "g"(y0)		\
		     : "cc");						\
    __asm__ volatile("adcl %5,%1; adcl %3,%0"				\
		     : "=r"(r3), "=r"(r2)				\
		     : "%0"(x3), "g"(y3), "%1"(x2), "g"(y2)		\
		     : "cc");						\
  } while (0)

#define __FP_FRAC_SUB_2(rh, rl, xh, xl, yh, yl)				\
  __asm__("subl %5,%1; sbbl %4,%0"					\
	  : "=r"(rh), "=r"(rl)						\
	  : "0"(xh), "1"(xl), "g"(yh), "g"(yl)				\
	  : "cc")

#define __FP_CLZ(r, x)							\
  do {									\
    __asm__("bsrl %1,%0" : "=r"(r) : "g"(x) : "cc");			\
    r ^= 31;								\
  } while (0)

#define _i386_mul_32_64(rh, rl, x, y)					\
  __asm__("mull %2" : "=d"(rh), "=a"(rl) : "%g"(x), "1"(y) : "cc")

#define _i386_div_64_32(q, r, nh, nl, d)				\
  __asm__ ("divl %4" : "=a"(q), "=d"(r) : "0"(nl), "1"(nh), "g"(d) : "cc")


#define _FP_MUL_MEAT_S(R,X,Y)					\
  _FP_MUL_MEAT_1_wide(_FP_WFRACBITS_S,R,X,Y,_i386_mul_32_64)
#define _FP_MUL_MEAT_D(R,X,Y)					\
  _FP_MUL_MEAT_2_wide(_FP_WFRACBITS_D,R,X,Y,_i386_mul_32_64)

#define _FP_DIV_MEAT_S(R,X,Y)	_FP_DIV_MEAT_1_udiv(S,R,X,Y)
#define _FP_DIV_MEAT_D(R,X,Y)	_FP_DIV_MEAT_2_udiv(D,R,X,Y)

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
