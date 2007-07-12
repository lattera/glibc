#define aio_fsync64 XXX
#include <aio.h>
/* And undo the hack.  */
#undef aio_fsync64
#include <kaio_misc.h>
#include <aio_fsync.c>
