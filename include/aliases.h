#ifndef _ALIASES_H
#include <inet/aliases.h>

extern int __getaliasent_r (struct aliasent *__restrict __result_buf,
			    char *__restrict __buffer, size_t __buflen,
			    struct aliasent **__restrict __result);
extern int __old_getaliasent_r (struct aliasent *__restrict __result_buf,
				char *__restrict __buffer, size_t __buflen,
				struct aliasent **__restrict __result);

extern int __getaliasbyname_r (__const char *__restrict __name,
			       struct aliasent *__restrict __result_buf,
			       char *__restrict __buffer, size_t __buflen,
			       struct aliasent **__restrict __result);
extern int __old_getaliasbyname_r (__const char *__restrict __name,
				   struct aliasent *__restrict __result_buf,
				   char *__restrict __buffer, size_t __buflen,
				   struct aliasent **__restrict __result);

#endif
