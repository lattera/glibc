#include <pwd/pwd.h>

/* Now define the internal interfaces.  */
extern int __getpwent_r __P ((struct passwd *__resultbuf, char *__buffer,
			      size_t __buflen, struct passwd **__result));
extern int __getpwuid_r __P ((__uid_t __uid, struct passwd *__resultbuf,
			      char *__buffer, size_t __buflen,
			      struct passwd **__result));
extern int __getpwnam_r __P ((__const char *__name, struct passwd *__resultbuf,
			      char *__buffer, size_t __buflen,
			      struct passwd **__result));
extern int __fgetpwent_r __P ((FILE * __stream, struct passwd *__resultbuf,
			       char *__buffer, size_t __buflen,
			       struct passwd **__result));
