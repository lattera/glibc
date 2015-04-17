/* We can compute the location of auxv without a loop, so we might as well
   pass it in.  */
#define LIBC_START_MAIN_AUXVEC_ARG
#include <csu/libc-start.c>
