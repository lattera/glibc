/* Linking in this module forces IEEE error handling rules for math functions.
   The default is POSIX.1 error handling.  */

#include <math.h>

_LIB_VERSION_TYPE _LIB_VERSION = _IEEE_;
