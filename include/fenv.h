#ifndef _FENV_H
#include <math/fenv.h>

#ifndef _ISOMAC
# include <stdbool.h>
/* Now define the internal interfaces.  */

extern int __feclearexcept (int __excepts);
extern int __fegetexcept (void);
extern int __fegetexceptflag (fexcept_t *__flagp, int __excepts);
extern int __feraiseexcept (int __excepts);
extern int __fesetexceptflag (const fexcept_t *__flagp, int __excepts);
extern int __fegetenv (fenv_t *__envp);
extern int __fesetenv (const fenv_t *__envp);
extern int __feupdateenv (const fenv_t *__envp);
extern __typeof (fegetround) __fegetround __attribute_pure__;
extern __typeof (feholdexcept) __feholdexcept;
extern __typeof (fesetround) __fesetround;

libm_hidden_proto (feraiseexcept)
libm_hidden_proto (__feraiseexcept)
libm_hidden_proto (fegetenv)
libm_hidden_proto (__fegetenv)
libm_hidden_proto (fegetround)
libm_hidden_proto (__fegetround)
libm_hidden_proto (fesetenv)
libm_hidden_proto (__fesetenv)
libm_hidden_proto (fesetround)
libm_hidden_proto (__fesetround)
libm_hidden_proto (feholdexcept)
libm_hidden_proto (__feholdexcept)
libm_hidden_proto (feupdateenv)
libm_hidden_proto (__feupdateenv)
libm_hidden_proto (fetestexcept)
libm_hidden_proto (feclearexcept)

/* Rounding mode context.  This allows functions to set/restore rounding mode
   only when the desired rounding mode is different from the current rounding
   mode.  */
struct rm_ctx
{
  fenv_t env;
  bool updated_status;
};

/* Track whether rounding mode macros were defined, since
   get-rounding-mode.h may define default versions if they weren't.
   FE_TONEAREST must always be defined (even if no changes of rounding
   mode are supported, glibc requires it to be defined to represent
   the default rounding mode).  */
# ifndef FE_TONEAREST
#  error "FE_TONEAREST not defined"
# endif
# if defined FE_DOWNWARD || defined FE_TOWARDZERO || defined FE_UPWARD
#  define FE_HAVE_ROUNDING_MODES 1
# else
#  define FE_HAVE_ROUNDING_MODES 0
# endif

#endif

#endif
