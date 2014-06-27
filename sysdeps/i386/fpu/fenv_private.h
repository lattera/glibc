#ifndef FENV_PRIVATE_H
#define FENV_PRIVATE_H 1

#include <fenv.h>
#include <fpu_control.h>

#ifdef __SSE2_MATH__
# define math_opt_barrier(x) \
  ({ __typeof(x) __x;					\
     if (sizeof (x) <= sizeof (double))			\
       __asm ("" : "=x" (__x) : "0" (x));		\
     else						\
       __asm ("" : "=t" (__x) : "0" (x));		\
     __x; })
# define math_force_eval(x) \
  do {							\
    if (sizeof (x) <= sizeof (double))			\
      __asm __volatile ("" : : "x" (x));		\
    else						\
      __asm __volatile ("" : : "f" (x));		\
  } while (0)
#else
# define math_opt_barrier(x) \
  ({ __typeof (x) __x;					\
     __asm ("" : "=t" (__x) : "0" (x));			\
     __x; })
# define math_force_eval(x) \
  do {							\
    __typeof (x) __x = (x);				\
    if (sizeof (x) <= sizeof (double))			\
      __asm __volatile ("" : : "m" (__x));		\
    else						\
      __asm __volatile ("" : : "f" (__x));		\
  } while (0)
#endif

/* This file is used by both the 32- and 64-bit ports.  The 64-bit port
   has a field in the fenv_t for the mxcsr; the 32-bit port does not.
   Instead, we (ab)use the only 32-bit field extant in the struct.  */
#ifndef __x86_64__
# define __mxcsr	__eip
#endif


/* All of these functions are private to libm, and are all used in pairs
   to save+change the fp state and restore the original state.  Thus we
   need not care for both the 387 and the sse unit, only the one we're
   actually using.  */

#if defined __AVX__ || defined SSE2AVX
# define STMXCSR "vstmxcsr"
# define LDMXCSR "vldmxcsr"
#else
# define STMXCSR "stmxcsr"
# define LDMXCSR "ldmxcsr"
#endif

static __always_inline void
libc_feholdexcept_sse (fenv_t *e)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = (mxcsr | 0x1f80) & ~0x3f;
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}

static __always_inline void
libc_feholdexcept_387 (fenv_t *e)
{
  /* Recall that fnstenv has a side-effect of masking exceptions.
     Clobber all of the fp registers so that the TOS field is 0.  */
  asm volatile ("fnstenv %0; fnclex"
		: "=m"(*e)
		: : "st", "st(1)", "st(2)", "st(3)",
		    "st(4)", "st(5)", "st(6)", "st(7)");
}

static __always_inline void
libc_fesetround_sse (int r)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  mxcsr = (mxcsr & ~0x6000) | (r << 3);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}

static __always_inline void
libc_fesetround_387 (int r)
{
  fpu_control_t cw;
  _FPU_GETCW (cw);
  cw = (cw & ~0xc00) | r;
  _FPU_SETCW (cw);
}

static __always_inline void
libc_feholdexcept_setround_sse (fenv_t *e, int r)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | (r << 3);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}

/* Set both rounding mode and precision.  A convenience function for use
   by libc_feholdexcept_setround and libc_feholdexcept_setround_53bit. */
static __always_inline void
libc_feholdexcept_setround_387_prec (fenv_t *e, int r)
{
  libc_feholdexcept_387 (e);

  fpu_control_t cw = e->__control_word;
  cw &= ~(_FPU_RC_ZERO | _FPU_EXTENDED);
  cw |= r | 0x3f;
  _FPU_SETCW (cw);
}

static __always_inline void
libc_feholdexcept_setround_387 (fenv_t *e, int r)
{
  libc_feholdexcept_setround_387_prec (e, r | _FPU_EXTENDED);
}

static __always_inline void
libc_feholdexcept_setround_387_53bit (fenv_t *e, int r)
{
  libc_feholdexcept_setround_387_prec (e, r | _FPU_DOUBLE);
}

static __always_inline int
libc_fetestexcept_sse (int e)
{
  unsigned int mxcsr;
  asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));
  return mxcsr & e & FE_ALL_EXCEPT;
}

static __always_inline int
libc_fetestexcept_387 (int ex)
{
  fexcept_t temp;
  asm volatile ("fnstsw %0" : "=a" (temp));
  return temp & ex & FE_ALL_EXCEPT;
}

static __always_inline void
libc_fesetenv_sse (fenv_t *e)
{
  asm volatile (LDMXCSR " %0" : : "m" (e->__mxcsr));
}

static __always_inline void
libc_fesetenv_387 (fenv_t *e)
{
  /* Clobber all fp registers so that the TOS value we saved earlier is
     compatible with the current state of the compiler.  */
  asm volatile ("fldenv %0"
		: : "m" (*e)
		: "st", "st(1)", "st(2)", "st(3)",
		  "st(4)", "st(5)", "st(6)", "st(7)");
}

static __always_inline int
libc_feupdateenv_test_sse (fenv_t *e, int ex)
{
  unsigned int mxcsr, old_mxcsr, cur_ex;
  asm volatile (STMXCSR " %0" : "=m" (*&mxcsr));
  cur_ex = mxcsr & FE_ALL_EXCEPT;

  /* Merge current exceptions with the old environment.  */
  old_mxcsr = e->__mxcsr;
  mxcsr = old_mxcsr | cur_ex;
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));

  /* Raise SIGFPE for any new exceptions since the hold.  Expect that
     the normal environment has all exceptions masked.  */
  if (__glibc_unlikely (~(old_mxcsr >> 7) & cur_ex))
    __feraiseexcept (cur_ex);

  /* Test for exceptions raised since the hold.  */
  return cur_ex & ex;
}

static __always_inline int
libc_feupdateenv_test_387 (fenv_t *e, int ex)
{
  fexcept_t cur_ex;

  /* Save current exceptions.  */
  asm volatile ("fnstsw %0" : "=a" (cur_ex));
  cur_ex &= FE_ALL_EXCEPT;

  /* Reload original environment.  */
  libc_fesetenv_387 (e);

  /* Merge current exceptions.  */
  __feraiseexcept (cur_ex);

  /* Test for exceptions raised since the hold.  */
  return cur_ex & ex;
}

static __always_inline void
libc_feupdateenv_sse (fenv_t *e)
{
  libc_feupdateenv_test_sse (e, 0);
}

static __always_inline void
libc_feupdateenv_387 (fenv_t *e)
{
  libc_feupdateenv_test_387 (e, 0);
}

static __always_inline void
libc_feholdsetround_sse (fenv_t *e, int r)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  e->__mxcsr = mxcsr;
  mxcsr = (mxcsr & ~0x6000) | (r << 3);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}

static __always_inline void
libc_feholdsetround_387_prec (fenv_t *e, int r)
{
  fpu_control_t cw;

  _FPU_GETCW (cw);
  e->__control_word = cw;
  cw &= ~(_FPU_RC_ZERO | _FPU_EXTENDED);
  cw |= r;
  _FPU_SETCW (cw);
}

static __always_inline void
libc_feholdsetround_387 (fenv_t *e, int r)
{
  libc_feholdsetround_387_prec (e, r | _FPU_EXTENDED);
}

static __always_inline void
libc_feholdsetround_387_53bit (fenv_t *e, int r)
{
  libc_feholdsetround_387_prec (e, r | _FPU_DOUBLE);
}

static __always_inline void
libc_feresetround_sse (fenv_t *e)
{
  unsigned int mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  mxcsr = (mxcsr & ~0x6000) | (e->__mxcsr & 0x6000);
  asm volatile (LDMXCSR " %0" : : "m" (*&mxcsr));
}

static __always_inline void
libc_feresetround_387 (fenv_t *e)
{
  _FPU_SETCW (e->__control_word);
}

#ifdef __SSE_MATH__
# define libc_feholdexceptf		libc_feholdexcept_sse
# define libc_fesetroundf		libc_fesetround_sse
# define libc_feholdexcept_setroundf	libc_feholdexcept_setround_sse
# define libc_fetestexceptf		libc_fetestexcept_sse
# define libc_fesetenvf			libc_fesetenv_sse
# define libc_feupdateenv_testf		libc_feupdateenv_test_sse
# define libc_feupdateenvf		libc_feupdateenv_sse
# define libc_feholdsetroundf		libc_feholdsetround_sse
# define libc_feresetroundf		libc_feresetround_sse
#else
# define libc_feholdexceptf		libc_feholdexcept_387
# define libc_fesetroundf		libc_fesetround_387
# define libc_feholdexcept_setroundf	libc_feholdexcept_setround_387
# define libc_fetestexceptf		libc_fetestexcept_387
# define libc_fesetenvf			libc_fesetenv_387
# define libc_feupdateenv_testf		libc_feupdateenv_test_387
# define libc_feupdateenvf		libc_feupdateenv_387
# define libc_feholdsetroundf		libc_feholdsetround_387
# define libc_feresetroundf		libc_feresetround_387
#endif /* __SSE_MATH__ */

#ifdef __SSE2_MATH__
# define libc_feholdexcept		libc_feholdexcept_sse
# define libc_fesetround		libc_fesetround_sse
# define libc_feholdexcept_setround	libc_feholdexcept_setround_sse
# define libc_fetestexcept		libc_fetestexcept_sse
# define libc_fesetenv			libc_fesetenv_sse
# define libc_feupdateenv_test		libc_feupdateenv_test_sse
# define libc_feupdateenv		libc_feupdateenv_sse
# define libc_feholdsetround		libc_feholdsetround_sse
# define libc_feresetround		libc_feresetround_sse
#else
# define libc_feholdexcept		libc_feholdexcept_387
# define libc_fesetround		libc_fesetround_387
# define libc_feholdexcept_setround	libc_feholdexcept_setround_387
# define libc_fetestexcept		libc_fetestexcept_387
# define libc_fesetenv			libc_fesetenv_387
# define libc_feupdateenv_test		libc_feupdateenv_test_387
# define libc_feupdateenv		libc_feupdateenv_387
# define libc_feholdsetround		libc_feholdsetround_387
# define libc_feresetround		libc_feresetround_387
#endif /* __SSE2_MATH__ */

#define libc_feholdexceptl		libc_feholdexcept_387
#define libc_fesetroundl		libc_fesetround_387
#define libc_feholdexcept_setroundl	libc_feholdexcept_setround_387
#define libc_fetestexceptl		libc_fetestexcept_387
#define libc_fesetenvl			libc_fesetenv_387
#define libc_feupdateenv_testl		libc_feupdateenv_test_387
#define libc_feupdateenvl		libc_feupdateenv_387
#define libc_feholdsetroundl		libc_feholdsetround_387
#define libc_feresetroundl		libc_feresetround_387

#ifndef __SSE2_MATH__
# define libc_feholdexcept_setround_53bit libc_feholdexcept_setround_387_53bit
# define libc_feholdsetround_53bit	libc_feholdsetround_387_53bit
#endif

/* We have support for rounding mode context.  */
#define HAVE_RM_CTX 1

static __always_inline void
libc_feholdexcept_setround_sse_ctx (struct rm_ctx *ctx, int r)
{
  unsigned int mxcsr, new_mxcsr;
  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  new_mxcsr = ((mxcsr | 0x1f80) & ~0x603f) | (r << 3);

  ctx->env.__mxcsr = mxcsr;
  if (__glibc_unlikely (mxcsr != new_mxcsr))
    {
      asm volatile (LDMXCSR " %0" : : "m" (*&new_mxcsr));
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

/* Unconditional since we want to overwrite any exceptions that occurred in the
   context.  This is also why all fehold* functions unconditionally write into
   ctx->env.  */
static __always_inline void
libc_fesetenv_sse_ctx (struct rm_ctx *ctx)
{
  libc_fesetenv_sse (&ctx->env);
}

static __always_inline void
libc_feupdateenv_sse_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    libc_feupdateenv_test_sse (&ctx->env, 0);
}

static __always_inline void
libc_feholdexcept_setround_387_prec_ctx (struct rm_ctx *ctx, int r)
{
  libc_feholdexcept_387 (&ctx->env);

  fpu_control_t cw = ctx->env.__control_word;
  fpu_control_t old_cw = cw;
  cw &= ~(_FPU_RC_ZERO | _FPU_EXTENDED);
  cw |= r | 0x3f;

  if (__glibc_unlikely (old_cw != cw))
    {
      _FPU_SETCW (cw);
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

static __always_inline void
libc_feholdexcept_setround_387_ctx (struct rm_ctx *ctx, int r)
{
  libc_feholdexcept_setround_387_prec_ctx (ctx, r | _FPU_EXTENDED);
}

static __always_inline void
libc_feholdexcept_setround_387_53bit_ctx (struct rm_ctx *ctx, int r)
{
  libc_feholdexcept_setround_387_prec_ctx (ctx, r | _FPU_DOUBLE);
}

static __always_inline void
libc_feholdsetround_387_prec_ctx (struct rm_ctx *ctx, int r)
{
  fpu_control_t cw, new_cw;

  _FPU_GETCW (cw);
  new_cw = cw;
  new_cw &= ~(_FPU_RC_ZERO | _FPU_EXTENDED);
  new_cw |= r;

  ctx->env.__control_word = cw;
  if (__glibc_unlikely (new_cw != cw))
    {
      _FPU_SETCW (new_cw);
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

static __always_inline void
libc_feholdsetround_387_ctx (struct rm_ctx *ctx, int r)
{
  libc_feholdsetround_387_prec_ctx (ctx, r | _FPU_EXTENDED);
}

static __always_inline void
libc_feholdsetround_387_53bit_ctx (struct rm_ctx *ctx, int r)
{
  libc_feholdsetround_387_prec_ctx (ctx, r | _FPU_DOUBLE);
}

static __always_inline void
libc_feholdsetround_sse_ctx (struct rm_ctx *ctx, int r)
{
  unsigned int mxcsr, new_mxcsr;

  asm (STMXCSR " %0" : "=m" (*&mxcsr));
  new_mxcsr = (mxcsr & ~0x6000) | (r << 3);

  ctx->env.__mxcsr = mxcsr;
  if (__glibc_unlikely (new_mxcsr != mxcsr))
    {
      asm volatile (LDMXCSR " %0" : : "m" (*&new_mxcsr));
      ctx->updated_status = true;
    }
  else
    ctx->updated_status = false;
}

static __always_inline void
libc_feresetround_sse_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    libc_feresetround_sse (&ctx->env);
}

static __always_inline void
libc_feresetround_387_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    _FPU_SETCW (ctx->env.__control_word);
}

static __always_inline void
libc_feupdateenv_387_ctx (struct rm_ctx *ctx)
{
  if (__glibc_unlikely (ctx->updated_status))
    libc_feupdateenv_test_387 (&ctx->env, 0);
}

#ifdef __SSE_MATH__
# define libc_feholdexcept_setroundf_ctx libc_feholdexcept_setround_sse_ctx
# define libc_fesetenvf_ctx		libc_fesetenv_sse_ctx
# define libc_feupdateenvf_ctx		libc_feupdateenv_sse_ctx
# define libc_feholdsetroundf_ctx	libc_feholdsetround_sse_ctx
# define libc_feresetroundf_ctx		libc_feresetround_sse_ctx
#else
# define libc_feholdexcept_setroundf_ctx libc_feholdexcept_setround_387_ctx
# define libc_feupdateenvf_ctx		libc_feupdateenv_387_ctx
# define libc_feholdsetroundf_ctx	libc_feholdsetround_387_ctx
# define libc_feresetroundf_ctx		libc_feresetround_387_ctx
#endif /* __SSE_MATH__ */

#ifdef __SSE2_MATH__
# define libc_feholdexcept_setround_ctx	libc_feholdexcept_setround_sse_ctx
# define libc_fesetenv_ctx		libc_fesetenv_sse_ctx
# define libc_feupdateenv_ctx		libc_feupdateenv_sse_ctx
# define libc_feholdsetround_ctx	libc_feholdsetround_sse_ctx
# define libc_feresetround_ctx		libc_feresetround_sse_ctx
#else
# define libc_feholdexcept_setround_ctx	libc_feholdexcept_setround_387_ctx
# define libc_feupdateenv_ctx		libc_feupdateenv_387_ctx
# define libc_feholdsetround_ctx	libc_feholdsetround_387_ctx
# define libc_feresetround_ctx		libc_feresetround_387_ctx
#endif /* __SSE2_MATH__ */

#define libc_feholdexcept_setroundl_ctx	libc_feholdexcept_setround_387_ctx
#define libc_feupdateenvl_ctx		libc_feupdateenv_387_ctx
#define libc_feholdsetroundl_ctx	libc_feholdsetround_387_ctx
#define libc_feresetroundl_ctx		libc_feresetround_387_ctx

#ifndef __SSE2_MATH__
# define libc_feholdsetround_53bit_ctx	libc_feholdsetround_387_53bit_ctx
# define libc_feresetround_53bit_ctx	libc_feresetround_387_ctx
#endif

#undef __mxcsr

#endif /* FENV_PRIVATE_H */
