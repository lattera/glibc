/* Timer test using the monotonic clock.  */

#include <time.h>
#include <unistd.h>

#if defined CLOCK_MONOTONIC && defined _POSIX_MONOTONIC_CLOCK
# define TEST_CLOCK	CLOCK_MONOTONIC
# define TEST_CLOCK_MISSING(clock) \
  (sysconf (_SC_MONOTONIC_CLOCK) > 0 ? NULL : #clock)
# include "tst-timer4.c"
#else
# define TEST_FUNCTION	0
# include "../test-skeleton.c"
#endif
