#include <resolv/resolv.h>

/* Internal interfaces.  */

extern int __ns_name_ntop (const u_char *, char *, size_t);
extern int __ns_name_unpack (const u_char *, const u_char *,
			     const u_char *, u_char *, size_t);
