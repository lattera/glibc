#ifndef _SHADOW_H
#include <shadow/shadow.h>

/* Now define the internal interfaces.  */
extern int __getspent_r (struct spwd *__result_buf, char *__buffer,
			 size_t __buflen, struct spwd **__result);
extern int __getspnam_r (__const char *__name, struct spwd *__result_buf,
			 char *__buffer, size_t __buflen,
			 struct spwd **__result);
extern int __sgetspent_r (__const char *__string,
			  struct spwd *__result_buf, char *__buffer,
			  size_t __buflen, struct spwd **__result);
extern int __fgetspent_r (FILE *__stream, struct spwd *__result_buf,
			  char *__buffer, size_t __buflen,
			  struct spwd **__result);
extern int __lckpwdf (void);
extern int __ulckpwdf (void);
#endif
