#include <posix/sys/times.h>

/* Now define the internal interfaces.  */
extern clock_t __times __P ((struct tms *__buffer));
