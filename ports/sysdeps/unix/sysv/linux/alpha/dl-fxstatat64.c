/* In this implementation we do not really care whether the call fails
   because of missing kernel support since we do not even call the
   function in this case.  */
/* For Alpha, in <kernel-features.h> we redefine the default definition of
   when __ASSUME_ATFCTS is present.  The hack must wait until after that.  */
#include <kernel-features.h>
#undef __ASSUME_ATFCTS
#define __ASSUME_ATFCTS 1
#include "fxstatat.c"
