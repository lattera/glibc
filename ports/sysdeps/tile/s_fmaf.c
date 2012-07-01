/* tile does not support the rounding modes required by the ieee754/dbl-64
   implementation, but it's still better than the generic implementation.  */

#define libc_feholdexcept_setround(e, x) do { (void) (e); } while (0)
#define libc_feupdateenv_test(e, x) ((void) (e), 0)
#define libc_fetestexcept(x) 0

#include <sysdeps/ieee754/dbl-64/s_fmaf.c>
