#ifndef _LIBINTL_H
#include <intl/libintl.h>

/* Now define the internal interfaces.  */
extern char *__gettext __P ((__const char *__msgid));
extern char *__textdomain __P ((__const char *__domainname));
extern char *__bindtextdomain __P ((__const char *__domainname,
				    __const char *__dirname));
#endif
