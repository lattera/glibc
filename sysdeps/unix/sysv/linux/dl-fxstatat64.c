/* In this implementation we do not really care whether the call fails
   because of missing kernel support since we do not even call the
   function in this case.  */
#undef __ASSUME_ATFCTS
#define __ASSUME_ATFCTS 1
#include "fxstatat64.c"
