#ifndef FENV_PRIVATE_H
#define FENV_PRIVATE_H 1

#include <fenv.h>

static __always_inline void
libc_feholdexcept (fenv_t *e)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  *(e) = etmp;
  etmp = etmp & ~((0x1f << 23) | FE_ALL_EXCEPT);
  __fenv_ldfsr(etmp);
}

static __always_inline void
libc_fesetround (int r)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  etmp = (etmp & ~__FE_ROUND_MASK) | (r);
  __fenv_ldfsr(etmp);
}

static __always_inline void
libc_feholdexcept_setround (fenv_t *e, int r)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  *(e) = etmp;
  etmp = etmp & ~((0x1f << 23) | FE_ALL_EXCEPT);
  etmp = (etmp & ~__FE_ROUND_MASK) | (r);
  __fenv_ldfsr(etmp);
}

static __always_inline int
libc_fetestexcept (int e)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  return etmp & (e) & FE_ALL_EXCEPT;
}

static __always_inline void
libc_fesetenv (fenv_t *e)
{
  __fenv_ldfsr(*e);
}

static __always_inline int
libc_feupdateenv_test (fenv_t *e, int ex)
{
  fenv_t etmp;

  __fenv_stfsr(etmp);
  etmp &= FE_ALL_EXCEPT;

  __fenv_ldfsr(*e);

  __feraiseexcept (etmp);

  return etmp & ex;
}

static __always_inline void
libc_feupdateenv (fenv_t *e)
{
  libc_feupdateenv_test (e, 0);
}

static __always_inline void
libc_feholdsetround (fenv_t *e, int r)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  *(e) = etmp;
  etmp = (etmp & ~__FE_ROUND_MASK) | (r);
  __fenv_ldfsr(etmp);
}

static __always_inline void
libc_feresetround (fenv_t *e)
{
  fenv_t etmp;
  __fenv_stfsr(etmp);
  etmp = (etmp & ~__FE_ROUND_MASK) | (*e & __FE_ROUND_MASK);
  __fenv_ldfsr(etmp);
}

#define libc_feholdexceptf		libc_feholdexcept
#define libc_fesetroundf		libc_fesetround
#define libc_feholdexcept_setroundf	libc_feholdexcept_setround
#define libc_fetestexceptf		libc_fetestexcept
#define libc_fesetenvf			libc_fesetenv
#define libc_feupdateenv_testf		libc_feupdateenv_test
#define libc_feupdateenvf		libc_feupdateenv
#define libc_feholdsetroundf		libc_feholdsetround
#define libc_feresetroundf		libc_feresetround
#define libc_feholdexcept		libc_feholdexcept
#define libc_fesetround			libc_fesetround
#define libc_feholdexcept_setround	libc_feholdexcept_setround
#define libc_fetestexcept		libc_fetestexcept
#define libc_fesetenv			libc_fesetenv
#define libc_feupdateenv_test		libc_feupdateenv_test
#define libc_feupdateenv		libc_feupdateenv
#define libc_feholdsetround		libc_feholdsetround
#define libc_feresetround		libc_feresetround
#define libc_feholdexceptl		libc_feholdexcept
#define libc_fesetroundl		libc_fesetround
#define libc_feholdexcept_setroundl	libc_feholdexcept_setround
#define libc_fetestexceptl		libc_fetestexcept
#define libc_fesetenvl			libc_fesetenv
#define libc_feupdateenv_testl		libc_feupdateenv_test
#define libc_feupdateenvl		libc_feupdateenv
#define libc_feholdsetroundl		libc_feholdsetround
#define libc_feresetroundl		libc_feresetround

#endif /* FENV_PRIVATE_H */
