/* Define a constant for the dgettext domainname for libc internal messages,
   so the string constant is not repeated in dozens of object files.  */

#include "../version.h"

const char _libc_intl_domainname[] = "SYS_GNU_libc-" VERSION;
