#include <sysdep.h>

/* Always multi-thread (since there's at least the sig handler), but no
   handling enabled.  */
#define SINGLE_THREAD_P (0)
#define RTLD_SINGLE_THREAD_P (0)
#define LIBC_CANCEL_ASYNC()	0 /* Just a dummy value.  */
#define LIBC_CANCEL_RESET(val)	((void)(val)) /* Nothing, but evaluate it.  */
#define LIBC_CANCEL_HANDLED()	/* Nothing.  */
