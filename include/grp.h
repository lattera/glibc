#ifndef _GRP_H
#include <grp/grp.h>

/* Now define the internal interfaces.  */
extern int __getgrent_r __P ((struct group *__resultbuf, char *buffer,
			      size_t __buflen, struct group **__result));
extern int __fgetgrent_r __P ((FILE * __stream, struct group *__resultbuf,
			       char *buffer, size_t __buflen,
			       struct group **__result));

/* Search for an entry with a matching group ID.  */
extern int __getgrgid_r __P ((__gid_t __gid, struct group *__resultbuf,
			      char *__buffer, size_t __buflen,
			      struct group **__result));

/* Search for an entry with a matching group name.  */
extern int __getgrnam_r __P ((__const char *__name, struct group *__resultbuf,
			      char *__buffer, size_t __buflen,
			      struct group **__result));
#endif
