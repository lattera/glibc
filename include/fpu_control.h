#include_next <fpu_control.h>

/* Called at startup.  It can be used to manipulate fpu control register.  */
extern void __setfpucw (fpu_control_t);
