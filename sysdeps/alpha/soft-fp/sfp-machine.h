#define _FP_W_TYPE_SIZE		64
#define _FP_W_TYPE		unsigned long
#define _FP_WS_TYPE		signed long
#define _FP_I_TYPE		long

#define _alpha_mul_64_128(rhi, rlo, x, y)		\
  __asm__("umulh %2,%3,%0; mulq %2,%3,%1"		\
	  : "=&r"(rhi), "=r"(rlo) : "r"(x), "r"(y))

#define _FP_MUL_MEAT_S(R,X,Y)					\
  _FP_MUL_MEAT_1_imm(_FP_WFRACBITS_S,R,X,Y)
#define _FP_MUL_MEAT_D(R,X,Y)					\
  _FP_MUL_MEAT_1_wide(_FP_WFRACBITS_D,R,X,Y,_alpha_mul_64_128)
#define _FP_MUL_MEAT_Q(R,X,Y)					\
  _FP_MUL_MEAT_2_wide(_FP_WFRACBITS_Q,R,X,Y,_alpha_mul_64_128)

#define _FP_DIV_MEAT_S(R,X,Y)	_FP_DIV_MEAT_1_imm(S,R,X,Y,_FP_DIV_HELP_imm)
#define _FP_DIV_MEAT_D(R,X,Y)	_FP_DIV_MEAT_1_udiv(D,R,X,Y)
#define _FP_DIV_MEAT_Q(R,X,Y)	_FP_DIV_MEAT_2_udiv(Q,R,X,Y)

#define _FP_NANFRAC_S		_FP_QNANBIT_S
#define _FP_NANFRAC_D		_FP_QNANBIT_D
#define _FP_NANFRAC_Q		_FP_QNANBIT_Q, 0
/* FIXME: This is just a wild guess */
#define _FP_NANSIGN_S		1
#define _FP_NANSIGN_D		1
#define _FP_NANSIGN_Q		1

#define _FP_KEEPNANFRACP 1
#define _FP_CHOOSENAN(fs, wc, R, X, Y, OP)			\
  do {								\
    R##_s = Y##_s;						\
    _FP_FRAC_COPY_##wc(R,Y);					\
    R##_c = FP_CLS_NAN;						\
  } while (0)
