#include "libioP.h"
#if _G_HAVE_ATEXIT
#include <stdlib.h>

typedef void (*voidfunc) __P((void));

/* Prototype.  */
static void DEFUN_VOID (_IO_register_cleanup);

static void
DEFUN_VOID(_IO_register_cleanup)
{
  atexit ((voidfunc)_IO_cleanup);
  _IO_cleanup_registration_needed = 0;
}

void (*_IO_cleanup_registration_needed) __P((void)) = _IO_register_cleanup;
#else
void (*_IO_cleanup_registration_needed) __P((void)) = NULL;
#endif /* _G_HAVE_ATEXIT */
