#include <mach/mach_types.h>
#include <mach/mach_port.h>

#define	SHORTCUT(name, number, args, typed_args)			      \
kern_return_t __##name typed_args					      \
{									      \
  kern_return_t ret = __syscall_##name args;				      \
  if (ret == MACH_SEND_INTERRUPTED)					      \
    ret = __mig_##name args;						      \
  return ret;								      \
}

#include "mach_shortcuts.h"
