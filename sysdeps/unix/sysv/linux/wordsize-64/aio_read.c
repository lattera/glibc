#define aio_read64 __renamed_aio_read64

#include "../../../../pthread/aio_read.c"

#undef aio_read64

weak_alias (aio_read, aio_read64)
