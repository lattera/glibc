#include <grp/grp.h>

/* Now define the internal interfaces.  */
extern int __getgrent_r __P ((struct group *__resultbuf, char *buffer,
			      size_t __buflen, struct group **__result));
extern int __fgetgrent_r __P ((FILE * __stream, struct group *__resultbuf,
			       char *buffer, size_t __buflen,
			       struct group **__result));
