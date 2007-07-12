/* We want an #include_next, but we are the main source file.
   So, #include ourselves and in that incarnation we can use #include_next.  */
#ifndef INCLUDED_SELF
# define INCLUDED_SELF
# include <sleep.c>
#else
/* This defines the CANCELLATION_P macro, which sleep.c checks for.  */
# include <pthreadP.h>
# include_next <sleep.c>
#endif
