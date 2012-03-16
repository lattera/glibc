#ifndef SPARC_MATH_PRIVATE_H
#define SPARC_MATH_PRIVATE_H 1

#include_next <math_private.h>
#include <fenv.h>

#undef libc_feholdexcept
#define libc_feholdexcept(e) \
  do {							\
     fenv_t etmp;					\
     __fenv_stfsr(etmp);				\
     *(e) = etmp;					\
     etmp = etmp & ~((0x1f << 23) | FE_ALL_EXCEPT);	\
     __fenv_ldfsr(etmp);				\
  } while (0)
#undef libc_feholdexceptf
#define libc_feholdexceptf(e) libc_feholdexcept (e)
#undef libc_feholdexceptl
#define libc_feholdexceptl(e) libc_feholdexcept (e)

#undef libc_feholdexcept_setround
#define libc_feholdexcept_setround(e, r) \
  do {							\
     fenv_t etmp;					\
     __fenv_stfsr(etmp);				\
     *(e) = etmp;					\
     etmp = etmp & ~((0x1f << 23) | FE_ALL_EXCEPT);	\
     etmp = (etmp & ~__FE_ROUND_MASK) | (r);		\
     __fenv_ldfsr(etmp);				\
  } while (0)
#undef libc_feholdexcept_setroundf
#define libc_feholdexcept_setroundf(e, r) libc_feholdexcept_setround (e, r)
#undef libc_feholdexcept_setroundl
#define libc_feholdexcept_setroundl(e, r) libc_feholdexcept_setround (e, r)

#undef libc_fetestexcept
#define libc_fetestexcept(e) \
  ({							\
     fenv_t etmp;					\
     __fenv_stfsr(etmp);				\
     etmp & (e) & FE_ALL_EXCEPT; })
#undef libc_fetestexceptf
#define libc_fetestexceptf(e) libc_fetestexcept (e)
#undef libc_fetestexceptl
#define libc_fetestexceptl(e) libc_fetestexcept (e)

#undef libc_fesetenv
#define libc_fesetenv(e) \
  __fenv_ldfsr(*e)
#undef libc_fesetenvf
#define libc_fesetenvf(e) libc_fesetenv (e)
#undef libc_fesetenvl
#define libc_fesetenvl(e) libc_fesetenv (e)

#undef libc_feupdateenv
#define libc_feupdateenv(e) \
  do {						\
     fenv_t etmp;				\
     __fenv_stfsr(etmp);			\
     __fenv_ldfsr(*e);				\
     __feraiseexcept (etmp & FE_ALL_EXCEPT);	\
  } while (0)
#undef libc_feupdateenvf
#define libc_feupdateenvf(e) libc_feupdateenv (e)
#undef libc_feupdateenvl
#define libc_feupdateenvl(e) libc_feupdateenv (e)

#endif /* SPARC_MATH_PRIVATE_H */
