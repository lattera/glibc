/* Include these first to avoid defining their inline functions.  */
#include <lock-intern.h>
#include <signal.h>

#undef _EXTERN_INLINE
#define _EXTERN_INLINE /* Define the real function. */

#include "hurd/fd.h"
#include "hurd/signal.h"
#include "hurd/userlink.h"
#include "hurd/threadvar.h"
#include "hurd/port.h"
